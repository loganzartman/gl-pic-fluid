layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform ivec3 grid_dim;
uniform vec3 bounds_min;
uniform vec3 bounds_max;

void main() {
    vec3 grid_size = bounds_max - bounds_min;
    uint index = gl_WorkGroupID.x;

    ivec3 grid_pos = ivec3(floor((particle[index].pos - bounds_min) / grid_size * grid_dim));
    if (grid_in_bounds(grid_dim, grid_pos)) {
        int gindex = grid_index(grid_dim, grid_pos);
        particle[index].vel = cell[gindex].vel;
    }
}

