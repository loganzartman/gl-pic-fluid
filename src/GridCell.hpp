#pragma once
#include <glm/glm.hpp>

const int GRID_AIR = 0;
const int GRID_FLUID = 1;

struct GridCell {
    glm::vec3 v;
    float pad0;
    int type;

    GridCell(const glm::vec3& v, int type) : v(v), type(type) {}
};
