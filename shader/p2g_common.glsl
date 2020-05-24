// requires atomic.glsl

struct P2GTransfer {
    AtomicFloatType u;
    AtomicFloatType v;
    AtomicFloatType w;
    AtomicFloatType weight_u;
    AtomicFloatType weight_v;
    AtomicFloatType weight_w;
    bool is_fluid;
};

layout(std430, binding=3) coherent buffer P2GTransferBlock {
    P2GTransfer p2g_transfer[];
};
