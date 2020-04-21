layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle {
    vec3 pos;
    float pad0;
    vec3 vel;
    float pad1;
    vec4 color;
};

layout(std430, binding=0) buffer ParticleBlock {
    Particle particle[];
};

void main() {
    uint index = gl_WorkGroupID.x;
    // particle[index].pos += particle[index].vel;
}
