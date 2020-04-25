int AIR = 0;
int SOLID = 1;
int FLUID = 2;

struct Particle {
    vec3 pos;
    float pad0;
    vec3 vel;
    float pad1;
    vec4 color;
};

struct GridCell {
    vec3 pos;
    int pad0;
    vec3 v;
    int pad1;
    int marker;
    int[3] pad2;
};

layout(std430, binding=0) restrict buffer ParticleBlock {
    Particle particle[];
};

layout(std430, binding=1) restrict buffer GridBlock {
    GridCell cell[];
};

int grid_index(ivec3 grid_dim, ivec3 grid_pos) {
    return grid_pos.z * (grid_dim.y * grid_dim.x) + grid_pos.y * grid_dim.x + grid_pos.x;
}

bool grid_in_bounds(ivec3 grid_dim, ivec3 grid_pos) {
    return grid_pos.x >= 0 && grid_pos.y >= 0 && grid_pos.z >= 0 &&
           grid_pos.x < grid_dim.x && grid_pos.y < grid_dim.y && grid_pos.z < grid_dim.y;
}
