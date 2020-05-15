layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    // TODO: will break for non-square grids
    float scale = dt / (density * cell_size.x);

    if (grid_pos.x > 0 && grid_pos.x < grid_dim.x - 2) {
        uint j = get_grid_index(grid_pos + ivec3(-1, 0, 0));
        if (cell[index].type == FLUID || cell[j].type == FLUID) {
            if (cell[index].type == SOLID || cell[j].type == SOLID) {
                cell[index].vel.x = 0;
            } else {
                cell[index].vel.x -= scale * (cell[index].pressure - cell[j].pressure);
            }
        }
    } else {
        // bordering solid
        if (cell[index].type == FLUID) {
            // cell[index].vel.x = 0;
        }
    }
    if (grid_pos.y > 0 && grid_pos.y < grid_dim.y - 2) {
        uint j = get_grid_index(grid_pos + ivec3(0, -1, 0));
        if (cell[index].type == FLUID || cell[j].type == FLUID) {
            if (cell[index].type == SOLID || cell[j].type == SOLID) {
                cell[index].vel.y = 0;
            } else {
                cell[index].vel.y -= scale * (cell[index].pressure - cell[j].pressure);
            }
        }
    } else {
        // bordering solid
        if (cell[index].type == FLUID) {
            // cell[index].vel.y = 0;
        }
    }
    if (grid_pos.z > 0 && grid_pos.z < grid_dim.z - 2) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, -1));
        if (cell[index].type == FLUID || cell[j].type == FLUID) {
            if (cell[index].type == SOLID || cell[j].type == SOLID) {
                cell[index].vel.z = 0;
            } else {
                cell[index].vel.z -= scale * (cell[index].pressure - cell[j].pressure);
            }
        }
    } else {
        // bordering solid
        if (cell[index].type == FLUID) {
            // cell[index].vel.z = 0;
        }
    }
}
