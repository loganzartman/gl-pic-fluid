#pragma once
#include <glm/glm.hpp>

class P2GTransfer {
    using byte4 = char[4]; // true type may differ in GLSL based on capabilities
    alignas(4) byte4 u;
    alignas(4) byte4 v;
    alignas(4) byte4 w;
    alignas(4) byte4 weight_u;
    alignas(4) byte4 weight_v;
    alignas(4) byte4 weight_w;
    alignas(4) bool is_fluid = false;
};
