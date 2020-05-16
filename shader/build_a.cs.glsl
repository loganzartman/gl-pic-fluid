void build_a() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    cell[index].pressure = 0;
    // warm start - don't clear guess

    cell[index].a_diag = 0;
    cell[index].a_x = 0;
    cell[index].a_y = 0;
    cell[index].a_z = 0;

    if (cell[index].type != FLUID) {
        return;
    }

    float scale = dt / (density * cell_size.x * cell_size.x);
    if (grid_pos.x > 0) {
        uint j = get_grid_index(grid_pos + ivec3(-1, 0, 0));
        if (cell[j].type == FLUID) {
            cell[index].a_diag += scale;
        }
    }
    if (grid_pos.x < grid_dim.x - 2) {
        uint j = get_grid_index(grid_pos + ivec3(1, 0, 0));
        if (cell[j].type == FLUID) {
            cell[index].a_diag += scale;
            cell[index].a_x = -scale;
        } else if (cell[j].type == AIR) {
            cell[index].a_diag += scale;
        }
    }
    if (grid_pos.y > 0) {
        uint j = get_grid_index(grid_pos + ivec3(0, -1, 0));
        if (cell[j].type == FLUID) {
            cell[index].a_diag += scale;
        }
    }
    if (grid_pos.y < grid_dim.y - 2) {
        uint j = get_grid_index(grid_pos + ivec3(0, 1, 0));
        if (cell[j].type == FLUID) {
            cell[index].a_diag += scale;
            cell[index].a_y = -scale;
        } else if (cell[j].type == AIR) {
            cell[index].a_diag += scale;
        }
    }
    if (grid_pos.z > 0) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, -1));
        if (cell[j].type == FLUID) {
            cell[index].a_diag += scale;
        }
    }
    if (grid_pos.z < grid_dim.z - 2) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, 1));
        if (cell[j].type == FLUID) {
            cell[index].a_diag += scale;
            cell[index].a_z = -scale;
        } else if (cell[j].type == AIR) {
            cell[index].a_diag += scale;
        }
    }
}
