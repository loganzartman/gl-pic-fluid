#pragma once
#include <glm/glm.hpp>

const int GRID_AIR = 0;
const int GRID_SOLID = 1;
const int GRID_FLUID = 2;

struct GridCell {
    alignas(16) glm::vec3 pos;
    alignas(4)  int marker;
    alignas(16) glm::vec3 v;

    GridCell(const glm::vec3& pos, const glm::vec3& v, int marker) : pos(pos), marker(marker), v(v) {}
};
