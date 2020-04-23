layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform ivec3 grid_dim;
uniform vec3 bounds_min;
uniform vec3 bounds_max;

void main() {
    vec3 bounds_size = bounds_max / bounds_min;
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    int index = grid_index(grid_dim, grid_pos);

    int marker = AIR;
    for (int i = 0; i < particle.length(); ++i) {
        vec3 pos = particle[i].pos;
        ivec3 p_grid_pos = ivec3(floor((pos - bounds_min) / bounds_size * grid_dim));
        if (p_grid_pos == grid_pos) {
            marker = FLUID;            
        }
    }
    cell[index].marker = marker;
}
