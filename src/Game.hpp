#include <array>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vec_swizzle.hpp>

#include "util.hpp"

class Game {
public:
    const float pi = glm::pi<float>();
    struct Particle {
        glm::vec3 pos;
        float pad0;
        glm::vec3 vel;
        float pad1;
        glm::vec4 color;

        Particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 color) : pos(pos), vel(vel), color(color) {}
    };

    const int num_circle_vertices = 8; // circle detail for particle rendering
    const int num_particles = 5000;

    GLFWwindow* window;
    glm::vec2 mouse_pos;
    glm::vec2 mouse_right_drag_start;
    bool mouse_right_dragging = false;

    const float camera_speed = 0.003;
    float camera_yaw;
    float camera_pitch;

    GLuint rect_vao;                  // VAO for background rectangle
    GLuint rect_program;              // program for background rectangle
    GLuint particles_ssbo;            // particle data storage
    GLuint particles_compute_program; // compute shader to operate on particles SSBO
    GLuint particles_vao;             // VAO for particle rendering
    GLuint particles_program;         // program for particle rendering

    Game(GLFWwindow* window) : window(window) {}

    void init() {
        create_rect_vao();
        create_rect_program();
        create_particles_ssbo();
        create_particles_compute_program();
        create_particles_vao();
        create_particles_program();
    }

    void create_rect_vao() {
        // create VAO, which encapsulates the vertex buffer and its layout
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object
        glGenVertexArrays(1, &rect_vao);
        glBindVertexArray(rect_vao);

        GLuint rect_vbo;
        glGenBuffers(1, &rect_vbo);

        glEnableVertexAttribArray(/* attribute index */ 0);

        // binds the VBO to a VAO attribute
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Separate_attribute_format
        glBindVertexBuffer(
            0,                // binding (attribute) index
            rect_vbo,         // buffer
            0,                // offset of first element
            sizeof(glm::vec3) // stride
        );
        // notice how we could bind different slices of the same buffer to different attributes using offset and stride.
        
        glVertexAttribFormat(
            0,                   // attribute index
            glm::vec3::length(), // size (number of components)
            GL_FLOAT,            // type of components
            GL_FALSE,            // normalize integer type?
            0                    // "added to the buffer binding's offset to get the offset for this attribute"
        );

        // could use glVertexBindingDivisor() to do instancing

        constexpr std::array<glm::vec3, 8> vbo_data{
            glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.0f, -1.f, -1.f), glm::vec3(1.0f, -1.f, 1.0f), 
            glm::vec3(-1.f, -1.f, 1.0f), glm::vec3(-1.f, 1.0f, 1.0f), glm::vec3(-1.f, 1.0f, -1.f),
            glm::vec3(1.0f, 1.0f, -1.f), glm::vec3(1.0f, 1.0f, 1.0f)
        };

        // load data into the VBO
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml
        glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,                     // target
            vbo_data.size() * sizeof(glm::vec3), // size in bytes of data
            vbo_data.data(),                     // pointer to data
            GL_STATIC_DRAW                       // usage (GL_STATIC_DRAW - unlikely to be updated)
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
        glBindVertexArray(0); // unbind
    }

    void create_rect_program() {
        constexpr char const* vert_source = R"(
            #version 430 core
            layout (location=0) in vec3 pos;
            out vec2 uv;

            uniform mat4 projection;
            uniform mat4 view;

            void main() {
                gl_Position = projection * view * vec4(pos, 1.0);
                uv = gl_Position.xy;
            }
        )";
        constexpr char const* frag_source = R"(
            #version 430 core 
            in vec2 uv;
            out vec4 frag_color;
            
            void main() {
                frag_color = vec4(1.0, 1.0, 1.0, 1.0);
            }
        )";

        // Create shader program object
        // https://www.khronos.org/opengl/wiki/GLSL_Object#Program_objects
        rect_program = glCreateProgram();

        // Create vertex shader program
        // https://www.khronos.org/opengl/wiki/Shader_Compilation#Shader_and_program_objects
        GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(
            vs_id,        // shader ID
            1,            // number of source strings
            &vert_source, // array of source strings
            NULL          // array of string lengths (NULL - null terminated)
        );
        glCompileShader(vs_id);
        glAttachShader(rect_program, vs_id);

        // Create fragment shader program
        GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(
            fs_id,        // shader ID
            1,            // number of source strings
            &frag_source, // array of source strings
            NULL          // array of string lengths (NULL - null terminated)
        );
        glCompileShader(fs_id);
        glAttachShader(rect_program, fs_id);

        // we could do more configuration before linking, but we will leave defaults
        // https://www.khronos.org/opengl/wiki/Shader_Compilation#Before_linking

        glLinkProgram(rect_program);
        check_program_errors(rect_program);
    }

    void create_particles_ssbo() {
        std::vector<Particle> initial;
        for (int i = 0; i < num_particles; ++i) {
            initial.emplace_back(Particle{
                glm::linearRand(glm::vec3(-1.0), glm::vec3(1.0)),
                glm::sphericalRand(0.001),
                glm::linearRand(glm::vec4(0.0), glm::vec4(1.0))
            });
        }

        // https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
        glGenBuffers(1, &particles_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particles_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, initial.size() * sizeof(Particle), initial.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    }

    void create_particles_compute_program() {
        constexpr char const* compute_src = R"(
            #version 430 core
            layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

            struct Particle {
                vec3 pos;
                float pad0;
                vec3 vel;
                float pad1;
                vec4 color;
            };

            layout(std430, binding=0) buffer ParticleBlock {
                Particle particle[];
            };

            void main() {
                uint index = gl_WorkGroupID.x;
                // particle[index].pos += particle[index].vel;
            }
        )";

        particles_compute_program = glCreateProgram();
        GLuint cs_id = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(cs_id, 1, &compute_src, NULL);
        glCompileShader(cs_id);
        glAttachShader(particles_compute_program, cs_id);
        glLinkProgram(particles_compute_program);
        check_program_errors(particles_compute_program);
    }

    void create_particles_vao() {
        // circle vertices (for triangle fan)
        std::vector<glm::vec2> circle;
        for (int i = 0; i < num_circle_vertices; ++i) {
            const float f = static_cast<float>(i) / num_circle_vertices * glm::pi<float>() * 2.0;
            circle.emplace_back(glm::vec2(glm::sin(f), glm::cos(f)));
        }

        glGenVertexArrays(1, &particles_vao);
        glBindVertexArray(particles_vao);

        // non-instanced circle geometry
        GLuint circle_vbo;
        glGenBuffers(1, &circle_vbo);

        glEnableVertexAttribArray(0);
        glBindVertexBuffer(0, circle_vbo, 0, sizeof(glm::vec2));
        glVertexAttribFormat(0, glm::vec2::length(), GL_FLOAT, GL_FALSE, 0);

        glBindBuffer(GL_ARRAY_BUFFER, circle_vbo);
        glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(glm::vec2), circle.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind

        // instanced particle position data
        glEnableVertexAttribArray(1);
        glBindVertexBuffer(1, particles_ssbo, offsetof(Particle, pos), sizeof(Particle));
        glVertexAttribFormat(1, glm::vec3::length(), GL_FLOAT, GL_FALSE, 0);
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribDivisor.xhtml
        glVertexAttribDivisor(1, 1); // one attribute value per instance

        // instanced particle color data
        glEnableVertexAttribArray(2);
        glBindVertexBuffer(2, particles_ssbo, offsetof(Particle, color), sizeof(Particle));
        glVertexAttribFormat(2, glm::vec4::length(), GL_FLOAT, GL_FALSE, 0);
        glVertexAttribDivisor(2, 1);

        glBindVertexArray(0); // unbind
    }

    void create_particles_program() {
        constexpr char const* vert_src = R"(
            #version 430 core
            layout(location=0) in vec2 circle_offset;
            layout(location=1) in vec3 particle_pos;
            layout(location=2) in vec4 particle_color;
            out vec4 color;
            uniform mat4 projection;
            uniform mat4 view;

            void main() {
                const float radius = 0.025;
                vec3 particle_pos_world = (view * vec4(particle_pos, 1.0)).xyz;
                vec3 vertex_pos_world = particle_pos_world + vec3(circle_offset, 0) * radius * 0.5;
                gl_Position = projection * vec4(vertex_pos_world, 1.0);
                color = particle_color;
            }
        )";
        constexpr char const* frag_src = R"(
            #version 430 core
            in vec4 color;
            out vec4 frag_color;

            void main() {
                frag_color = color;
            }
        )";

        particles_program = glCreateProgram();
        
        GLuint vert_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vert_id, 1, &vert_src, NULL);
        glCompileShader(vert_id);
        glAttachShader(particles_program, vert_id);

        GLuint frag_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(frag_id, 1, &frag_src, NULL);
        glCompileShader(frag_id);
        glAttachShader(particles_program, frag_id);

        glLinkProgram(particles_program);
        check_program_errors(particles_program);
    }

    void update_camera() {
        if (mouse_right_dragging) {
            const glm::vec2 dx = (mouse_pos - mouse_right_drag_start) * camera_speed;
            camera_yaw -= dx.x;
            camera_pitch -= dx.y;

            const float pitch_limit = pi/2-0.001;
            camera_pitch = glm::clamp(camera_pitch, -pitch_limit, pitch_limit);
            mouse_right_drag_start = mouse_pos;
        }
    }

    void update() {
        const double t = glfwGetTime();
        int window_w, window_h;
        glfwGetFramebufferSize(window, &window_w, &window_h);

        update_camera();

        // dispatch compute shader
        // https://www.khronos.org/opengl/wiki/Compute_Shader#Dispatch
        glUseProgram(particles_compute_program);
        glDispatchCompute(num_particles, 1, 1);
        glUseProgram(0); // unbind

        glViewport(0, 0, window_w, window_h);

        // projection matrix
        const float aspect_ratio = static_cast<double>(window_w) / window_h;
        const glm::mat4 projection = glm::perspective(glm::radians(30.f), aspect_ratio, 0.1f, 100.f);

        // view matrix (orbit camera) 
        glm::vec3 eye(0, 0, 5);
        eye = glm::rotate(eye, camera_yaw, glm::vec3(0, 1, 0));
        eye = glm::rotate(eye, camera_pitch, glm::cross(glm::vec3(0, 1, 0), eye));
        const glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        // clear screen
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.16, 0.14, 0.10, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // wait for compute shader
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // draw rectangle
        glUseProgram(rect_program);
        glUniformMatrix4fv(
            glGetUniformLocation(rect_program, "projection"), // uniform location
            1,                                                // count
            GL_FALSE,                                         // transpose
            glm::value_ptr(projection)                        // pointer to data
        );
        glUniformMatrix4fv(glGetUniformLocation(rect_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glBindVertexArray(rect_vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, std::array<uint, 36>{
            0, 3, 2, 2, 1, 0, 0, 5, 4, 4, 3, 0, 0, 1, 6, 6, 5, 0, 5, 6, 7, 7,
            4, 5, 1, 2, 7, 7, 6, 1, 3, 4, 7, 7, 2, 3
        }.data());
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0); // unbind
        glUseProgram(0); // unbind

        // draw particles
        glUseProgram(particles_program);
        glUniformMatrix4fv(glGetUniformLocation(particles_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(particles_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glBindVertexArray(particles_vao);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, num_particles);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};
