void build_a() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    cell[index].a_diag = 0;
    cell[index].a_x = 0;
    cell[index].a_y = 0;
    cell[index].a_z = 0;

    if (cell[index].type != FLUID) {
        return;
    }

    // TODO: for non square grid this will break
    float scale = dt / (density * cell_size.x * cell_size.x);

    int x = grid_pos.x;
    int y = grid_pos.y;
    int z = grid_pos.z;

    if (x > 0) {
        // x- neighbor
        uint i = get_grid_index(ivec3(x-1,y,z));
        if (cell[i].type == FLUID) {
            cell[index].a_diag += scale;
        }
    }
    if (x < grid_dim.x - 1) {
        // x+ neighbor
        uint i = get_grid_index(ivec3(x+1,y,z));
        if (cell[i].type == FLUID) {
            cell[index].a_diag += scale;
            cell[index].a_x = -scale;
        } else if (cell[i].type == AIR) {
            cell[index].a_diag += scale;
        }
    }
    if (y > 0) {
        // y- neighbor
        uint i = get_grid_index(ivec3(x,y-1,z));
        if (cell[i].type == FLUID) {
            cell[index].a_diag += scale;
        }
    }
    if (y < grid_dim.y - 1) {
        // y+ neighbor
        uint i = get_grid_index(ivec3(x,y+1,z));
        if (cell[i].type == FLUID) {
            cell[index].a_diag += scale;
            cell[index].a_y = -scale;
        } else if (cell[i].type == AIR) {
            cell[index].a_diag += scale;
        }
    }
    if (z > 0) {
        // z- neighbor
        uint i = get_grid_index(ivec3(x,y,z-1));
        if (cell[i].type == FLUID) {
            cell[index].a_diag += scale;
        }
    }
    if (z < grid_dim.z - 1) {
        // z+ neighbor
        uint i = get_grid_index(ivec3(x,y,z+1));
        if (cell[i].type == FLUID) {
            cell[index].a_diag += scale;
            cell[index].a_z = -scale;
        } else if (cell[i].type == AIR) {
            cell[index].a_diag += scale;
        }
    }
}
