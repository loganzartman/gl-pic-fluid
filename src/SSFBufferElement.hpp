#pragma once
#include <glm/glm.hpp>

struct SSFBufferElement {
    alignas(16) glm::vec3 eye_pos_a = glm::vec3(0);
    alignas(16) glm::vec3 eye_pos_b = glm::vec3(0);
};
