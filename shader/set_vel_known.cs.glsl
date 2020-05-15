layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    if (cell[index].vel_unknown == 2) {
        cell[index].vel_unknown = 0;
    }
}
