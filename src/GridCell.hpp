#pragma once
#include <glm/glm.hpp>

const int GRID_AIR = 0;
const int GRID_SOLID = 1;
const int GRID_FLUID = 2;

struct GridCell {
    alignas(16) glm::vec3 pos;
    alignas(4)  int type;
    alignas(16) glm::vec3 vel; // velocity component at each of three faces on cell cube (not a real vector)
    alignas(4)  float rhs = 0; // negative divergence for pressure solve

    alignas(16) glm::vec3 old_vel; // old velocity for FLIP update

    // elements of A matrix in pressure solve
    alignas(4)  float a_diag = 0;
    alignas(4)  float a_x = 0;
    alignas(4)  float a_y = 0;
    alignas(4)  float a_z = 0;

    alignas(4)  float pressure_guess = 0;
    alignas(4)  float pressure = 0;
    alignas(4)  int vel_unknown = 1;

    GridCell(const glm::vec3& pos, const glm::vec3& vel, int type) : pos(pos), type(type), vel(vel), old_vel(vel) {}
};
