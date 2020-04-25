#pragma once
#include <glm/glm.hpp>

const int GRID_AIR = 0;
const int GRID_SOLID = 1;
const int GRID_FLUID = 2;

struct GridCell {
    glm::vec3 pos;
    int pad0 = 0xbeef;
    glm::vec3 v;
    int pad1 = 0xbeef;
    int marker;
    int pad2[3] = {0xbeef};

    GridCell(const glm::vec3& pos, const glm::vec3& v, int marker) : pos(pos), v(v), marker(marker) {}
};
