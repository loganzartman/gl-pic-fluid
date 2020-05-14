#pragma once
#include <glm/glm.hpp>

const int GRID_AIR = 0;
const int GRID_SOLID = 1;
const int GRID_FLUID = 2;

struct GridCell {
    alignas(16) glm::vec3 pos;
    alignas(4)  int type;
    alignas(16) glm::vec3 vel; // velocity component at each of three faces on cell cube (not a real vector)
    alignas(4)  float rhs; // negative divergence for pressure solve 

    GridCell(const glm::vec3& pos, const glm::vec3& vel, int type) : pos(pos), type(type), vel(vel), rhs(0) {}
};
