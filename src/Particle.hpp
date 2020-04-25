#pragma once
#include <glm/glm.hpp>

struct Particle {
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec3 pos;
    alignas(16) glm::vec3 vel;

    Particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 color) : color(color), pos(pos), vel(vel) {}
};
