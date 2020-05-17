#pragma once
#include <glm/glm.hpp>

struct P2GTransfer {
    alignas(4) int u;
    alignas(4) int v;
    alignas(4) int w;

    P2GTransfer() : u(0), v(0), w(0) {}
};
