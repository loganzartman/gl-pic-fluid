void enforce_boundary_condition(ivec3 grid_pos) {
    uint index = get_grid_index(grid_pos);

    if (grid_pos.x == 0 || grid_pos.x == grid_dim.x - 1) {
        cell[index].vel.x = 0;
    }
    if (grid_pos.y == 0 || grid_pos.y == grid_dim.y - 1) {
        cell[index].vel.y = 0;
    }
    if (grid_pos.z == 0 || grid_pos.z == grid_dim.z - 1) {
        cell[index].vel.z = 0;
    }
}
