#pragma once
#include <glm/glm.hpp>

struct Particle {
    glm::vec3 pos;
    float pad0;
    glm::vec3 vel;
    float pad1;
    glm::vec4 color;

    Particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 color) : pos(pos), vel(vel), color(color) {}
};
