#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "util.hpp"
#include "gfx/object.hpp"

struct Box {
    gfx::Buffer vbo{GL_ARRAY_BUFFER};
    gfx::VAO vao;
    GLuint program;

    Box() {
        const std::vector<glm::vec3> data{
            glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.0f, -1.f, -1.f), glm::vec3(1.0f, -1.f, 1.0f), 
            glm::vec3(-1.f, -1.f, 1.0f), glm::vec3(-1.f, 1.0f, 1.0f), glm::vec3(-1.f, 1.0f, -1.f),
            glm::vec3(1.0f, 1.0f, -1.f), glm::vec3(1.0f, 1.0f, 1.0f)
        };
        vbo.set_data(data);
        vao.bind_attrib(vbo, 3, GL_FLOAT);
        create_program();
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
        vao.bind();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, std::array<uint, 36>{
            0, 3, 2, 2, 1, 0, 0, 5, 4, 4, 3, 0, 0, 1, 6, 6, 5, 0, 5, 6, 7, 7,
            4, 5, 1, 2, 7, 7, 6, 1, 3, 4, 7, 7, 2, 3
        }.data());
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        vao.unbind();
        glUseProgram(0); // unbind
    }
};
