#include <array>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Game {
public:
    GLFWwindow* window;
    glm::dvec2 mouse_pos;

    GLuint rect_vao;
    GLuint rect_program;

    Game(GLFWwindow* window) : window(window) {}

    void init() {
        create_rect_vao();
        create_rect_program();
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

            void main() {
                // gl_Position is probably deprecated; configure shaders correctly instead
                gl_Position = vec4(pos, 0.0, 1.0);
            }
        )";
        constexpr char const* frag_source = R"(
            #version 430 core 
            out vec4 frag_color;
            
            void main() {
                frag_color = vec4(1.0, 0.0, 0.0, 1.0);
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
    }

    void update() {
        int window_w, window_h;
        glfwGetFramebufferSize(window, &window_w, &window_h);

        glViewport(0, 0, window_w, window_h);

        // clear screen
        glClearColor(0.25, 0.45, 0.75, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw rectangle
        glUseProgram(rect_program);
        glBindVertexArray(rect_vao);
        glDrawArrays(GL_TRIANGLE_FAN, /* first */ 0, /* count */ 4);
        glBindVertexArray(0); // unbind
        glUseProgram(0); // unbind
    }
};
