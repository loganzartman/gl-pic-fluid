#pragma once
#include <glm/glm.hpp>

struct DebugLine {
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec3 a;
    alignas(16) glm::vec3 b;

    DebugLine(glm::vec3 a, glm::vec3 b, glm::vec4 color) : a(a), b(b), color(color) {}
};
