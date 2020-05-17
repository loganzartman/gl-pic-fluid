#pragma once
#include <glm/glm.hpp>

struct P2GTransfer {
    alignas(4) int u = 0;
    alignas(4) int v = 0;
    alignas(4) int w = 0;
    alignas(4) int weight_u = 0;
    alignas(4) int weight_v = 0;
    alignas(4) int weight_w = 0;
    alignas(4) bool is_fluid = false;
};
