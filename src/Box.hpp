#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "util.hpp"

struct Box {
    GLuint vao;
    GLuint program;

    Box() {
        create_vao();
        create_program();
    }

    void create_vao() {
        // create VAO, which encapsulates the vertex buffer and its layout
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo;
        glGenBuffers(1, &vbo);

        glEnableVertexAttribArray(/* attribute index */ 0);

        // binds the VBO to a VAO attribute
        // https://www.khronos.org/opengl/wiki/Vertex_Specification#Separate_attribute_format
        glBindVertexBuffer(
            0,                // binding (attribute) index
            vbo,         // buffer
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
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,                     // target
            vbo_data.size() * sizeof(glm::vec3), // size in bytes of data
            vbo_data.data(),                     // pointer to data
            GL_STATIC_DRAW                       // usage (GL_STATIC_DRAW - unlikely to be updated)
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind
        glBindVertexArray(0); // unbind
    }

    void create_program() {
        constexpr char const* vert_source = R"glsl(
            #version 430 core
            layout (location=0) in vec3 pos;
            out vec2 uv;

            uniform mat4 projection;
            uniform mat4 view;

            void main() {
                gl_Position = projection * view * vec4(pos, 1.0);
                uv = gl_Position.xy;
            }
        )glsl";
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
        program = glCreateProgram();

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
        glAttachShader(program, vs_id);

        // Create fragment shader program
        GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(
            fs_id,        // shader ID
            1,            // number of source strings
            &frag_source, // array of source strings
            NULL          // array of string lengths (NULL - null terminated)
        );
        glCompileShader(fs_id);
        glAttachShader(program, fs_id);

        // we could do more configuration before linking, but we will leave defaults
        // https://www.khronos.org/opengl/wiki/Shader_Compilation#Before_linking

        glLinkProgram(program);
        check_program_errors(program);
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) {
        glUseProgram(program);
        glUniformMatrix4fv(
            glGetUniformLocation(program, "projection"), // uniform location
            1,                                           // count
            GL_FALSE,                                    // transpose
            glm::value_ptr(projection)                   // pointer to data
        );
        glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glBindVertexArray(vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, std::array<uint, 36>{
            0, 3, 2, 2, 1, 0, 0, 5, 4, 4, 3, 0, 0, 1, 6, 6, 5, 0, 5, 6, 7, 7,
            4, 5, 1, 2, 7, 7, 6, 1, 3, 4, 7, 7, 2, 3
        }.data());
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0); // unbind
        glUseProgram(0); // unbind
    }
};
