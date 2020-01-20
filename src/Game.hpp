#include <array>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include "util.hpp"

class Game {
public:
    struct Particle {
        glm::vec2 pos;
        glm::vec2 vel;
    };

    const int num_circle_vertices = 16; // circle detail for particle rendering
    const int num_particles = 500;

    GLFWwindow* window;
    glm::dvec2 mouse_pos;

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
            sizeof(glm::vec2) // stride
        );
        // notice how we could bind different slices of the same buffer to different attributes using offset and stride.
        
        glVertexAttribFormat(
            0,                   // attribute index
            glm::vec2::length(), // size (number of components)
            GL_FLOAT,            // type of components
            GL_FALSE,            // normalize integer type?
            0                    // "added to the buffer binding's offset to get the offset for this attribute"
        );

        // could use glVertexBindingDivisor() to do instancing

        constexpr std::array<glm::vec2, 4> vbo_data{
            glm::vec2(0, 0),
            glm::vec2(1, 0),
            glm::vec2(1, 1),
            glm::vec2(0, 1)
        };

        // load data into the VBO
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBufferData.xhtml
        glBindBuffer(GL_ARRAY_BUFFER, rect_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,                     // target
            vbo_data.size() * sizeof(glm::vec2), // size in bytes of data
            vbo_data.data(),                     // pointer to data
            GL_STATIC_DRAW                       // usage (GL_STATIC_DRAW - unlikely to be updated)
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
        glBindVertexArray(0); // unbind
    }

    void create_rect_program() {
        constexpr char const* vert_source = R"(
            #version 430 core
            layout (location=0) in vec2 pos;
            out vec2 uv;

            void main() {
                // gl_Position is probably deprecated; configure shaders correctly instead
                gl_Position = vec4(pos, 0.0, 1.0);
                uv = pos;
            }
        )";
        constexpr char const* frag_source = R"(
            #version 430 core 
            in vec2 uv;
            out vec4 frag_color;
            
            struct Particle {
                vec2 pos;
                vec2 vel;
            };

            layout(std430, binding=0) buffer ParticleBlock {
                Particle particle[];
            };

            void main() {
                frag_color = vec4(uv, 0.0, 1.0);
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
                .pos = glm::linearRand(glm::vec2(0.3), glm::vec2(0.7)),
                .vel = glm::diskRand(0.001)
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
                vec2 pos;
                vec2 vel;
            };

            layout(std430, binding=0) buffer ParticleBlock {
                Particle particle[];
            };

            void main() {
                uint index = gl_WorkGroupID.x;
                particle[index].pos += particle[index].vel;
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
        glVertexAttribFormat(1, glm::vec2::length(), GL_FLOAT, GL_FALSE, 0);
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribDivisor.xhtml
        glVertexAttribDivisor(1, 1); // one attribute value per instance

        glBindVertexArray(0); // unbind
    }

    void create_particles_program() {
        constexpr char const* vert_src = R"(
            #version 430 core
            layout(location=0) in vec2 vertex_pos;
            layout(location=1) in vec2 particle_pos;

            void main() {
                const float radius = 0.05;
                gl_Position = vec4(particle_pos + vertex_pos * radius * 0.5, 0.0, 1.0);
            }
        )";
        constexpr char const* frag_src = R"(
            #version 430 core
            out vec4 frag_color;

            void main() {
                frag_color = vec4(0.0, 0.0, 1.0, 1.0);
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

    void update() {
        int window_w, window_h;
        glfwGetFramebufferSize(window, &window_w, &window_h);

        // dispatch compute shader
        // https://www.khronos.org/opengl/wiki/Compute_Shader#Dispatch
        glUseProgram(particles_compute_program);
        glDispatchCompute(num_particles, 1, 1);
        glUseProgram(0); // unbind

        glViewport(0, 0, window_w, window_h);

        // clear screen
        glClearColor(0.25, 0.45, 0.75, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // wait for compute shader
        // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // draw rectangle
        glUseProgram(rect_program);
        glBindVertexArray(rect_vao);
        glDrawArrays(GL_TRIANGLE_FAN, /* first */ 0, /* count */ 4);
        glBindVertexArray(0); // unbind
        glUseProgram(0); // unbind

        // draw particles
        glUseProgram(particles_program);
        glBindVertexArray(particles_vao);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_circle_vertices, num_particles);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};
