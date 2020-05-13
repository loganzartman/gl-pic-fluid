layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    if (grid_pos.x == 0 || grid_pos.x == grid_dim.x - 1) {
        cell[index].vel.x = 0;
    }
    if (grid_pos.y == 0 || grid_pos.y == grid_dim.y - 1) {
        cell[index].vel.y = 0;
    }
    if (grid_pos.z == 0 || grid_pos.z == grid_dim.z) {
        cell[index].vel.z = 0;
    }
}
