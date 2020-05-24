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
    vec3 old_vel;
    float a_diag;
    float a_x;
    float a_y;
    float a_z;
    float pressure_guess;
    float pressure;
    int vel_unknown;
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

uniform ivec3 grid_dim;
uniform vec3 bounds_min;
uniform vec3 bounds_max;
uniform ivec2 resolution;

ivec3 grid_cell_dim = grid_dim - ivec3(1);
vec3 bounds_size = bounds_max - bounds_min;
vec3 cell_size = bounds_size / vec3(grid_dim - ivec3(1));

const float density = 1; // kg/m^3

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

ivec3 offset_clamped(ivec3 base_coord, ivec3 dimension_offset) {
    // apply an offset (in one basis direction) and clamp to MAC grid
    ivec3 max_size = grid_cell_dim;
    if (dimension_offset.x > 0)
        max_size.x = grid_dim.x;
    if (dimension_offset.y > 0)
        max_size.y = grid_dim.y;
    if (dimension_offset.z > 0)
        max_size.z = grid_dim.z;
    return clamp(base_coord + dimension_offset, ivec3(0), max_size - ivec3(1));
}
