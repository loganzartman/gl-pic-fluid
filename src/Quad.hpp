#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "gfx/object.hpp"

class Quad {
public:
    gfx::Buffer buffer{GL_ARRAY_BUFFER};
    gfx::VAO vao;

    Quad() {create();}

    void create() {
        std::vector<glm::vec2> quad;
        quad.emplace_back(0, 0);
        quad.emplace_back(1, 0);
        quad.emplace_back(1, 1);
        quad.emplace_back(0, 1);
        buffer.set_data(quad);
        vao.bind_attrib(buffer, 2, GL_FLOAT);
    }

    void draw() {
        vao.bind();
        glDrawArrays(GL_TRIANGLE_FAN, 0, buffer.length());
        vao.unbind();
    }
};
