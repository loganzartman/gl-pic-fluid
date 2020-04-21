#pragma once
#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "util.hpp"
#include "gfx/object.hpp"
#include "gfx/program.hpp"

struct Box {
    gfx::Buffer vbo{GL_ARRAY_BUFFER};
    gfx::VAO vao;
    gfx::Program program;

    Box() {
        const std::vector<glm::vec3> data{
            glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.0f, -1.f, -1.f), glm::vec3(1.0f, -1.f, 1.0f), 
            glm::vec3(-1.f, -1.f, 1.0f), glm::vec3(-1.f, 1.0f, 1.0f), glm::vec3(-1.f, 1.0f, -1.f),
            glm::vec3(1.0f, 1.0f, -1.f), glm::vec3(1.0f, 1.0f, 1.0f)
        };
        vbo.set_data(data);
        vao.bind_attrib(vbo, 3, GL_FLOAT);
        program.vertex({"box.vs.glsl"}).fragment({"box.fs.glsl"}).compile();
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) {
        program.use();
        glUniformMatrix4fv(
            program.uniform_loc("projection"), 
            1,                                 // count
            GL_FALSE,                          // transpose
            glm::value_ptr(projection)         // pointer to data
        );
        glUniformMatrix4fv(program.uniform_loc("view"), 1, GL_FALSE, glm::value_ptr(view));
        vao.bind();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, std::array<uint, 36>{
            0, 3, 2, 2, 1, 0, 0, 5, 4, 4, 3, 0, 0, 1, 6, 6, 5, 0, 5, 6, 7, 7,
            4, 5, 1, 2, 7, 7, 6, 1, 3, 4, 7, 7, 2, 3
        }.data());
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        vao.unbind();
    }
};
