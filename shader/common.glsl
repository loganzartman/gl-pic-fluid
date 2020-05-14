int AIR = 0;
int SOLID = 1;
int FLUID = 2;

struct Particle {
    vec4 color;
    vec3 pos;
    vec3 vel;
};

struct GridCell {
    vec3 pos;
    int type;
    vec3 vel;
    float rhs;
    float a_diag;
    float a_x;
    float a_y;
    float a_z;
    float pressure_guess;
    float pressure;
};

struct DebugLine {
    vec4 color;
    vec3 a;
    vec3 b;
};

layout(std430, binding=0) restrict buffer ParticleBlock {
    Particle particle[];
};

layout(std430, binding=1) restrict buffer GridBlock {
    GridCell cell[];
};

layout(std430, binding=2) restrict buffer DebugLinesBlock {
    DebugLine debug_lines[];
};

const float density = 1; // kg/m^3
uniform ivec3 grid_dim;
ivec3 grid_cell_dim = grid_dim - ivec3(1);
uniform vec3 bounds_min;
uniform vec3 bounds_max;
vec3 bounds_size = bounds_max - bounds_min;
vec3 cell_size = bounds_size / vec3(grid_dim - ivec3(1));

bool grid_in_bounds(ivec3 grid_coord) {
    return grid_coord.x >= 0 && grid_coord.y >= 0 && grid_coord.z >= 0 &&
           grid_coord.x < grid_dim.x && grid_coord.y < grid_dim.y && grid_coord.z < grid_dim.y;
}

ivec3 get_grid_coord(vec3 pos, ivec3 half_offset) {
    return ivec3(floor((pos + vec3(half_offset) * (cell_size / 2.0) - bounds_min) / bounds_size * vec3(grid_cell_dim)));
}

vec3 get_world_coord(ivec3 grid_coord, ivec3 half_offset) {
    return bounds_min + vec3(grid_coord) * cell_size + vec3(half_offset) * cell_size * 0.5;
}

int get_grid_index(ivec3 grid_coord) {
    ivec3 clamped_coord = clamp(grid_coord, ivec3(0), grid_dim - ivec3(1));
    return grid_coord.z * grid_dim.y * grid_dim.x + grid_coord.y * grid_dim.x + grid_coord.x;
}
