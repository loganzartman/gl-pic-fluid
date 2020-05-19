layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    // TODO: will break for non-square grids
    float scale = dt / (density * cell_size.x);

    if (cell[index].type == FLUID || cell[get_grid_index(grid_pos + ivec3(-1, 0, 0))].type == FLUID) {
        // check solid
        if (grid_pos.x == 0 || grid_pos.x == grid_dim.x - 1) {
            cell[index].vel.x = 0;
        } else {
            cell[index].vel.x -= scale * (cell[index].pressure - cell[get_grid_index(grid_pos + ivec3(-1, 0, 0))].pressure);
        }
    } else {
        cell[index].vel_unknown = 1;
    }

    if (cell[index].type == FLUID || cell[get_grid_index(grid_pos + ivec3(0, -1, 0))].type == FLUID) {
        // check solid
        if (grid_pos.y == 0 || grid_pos.y == grid_dim.y - 1) {
            cell[index].vel.y = 0;
        } else {
            cell[index].vel.y -= scale * (cell[index].pressure - cell[get_grid_index(grid_pos + ivec3(0, -1, 0))].pressure);
        }
    } else {
        cell[index].vel_unknown = 1;
    }

    if (cell[index].type == FLUID || cell[get_grid_index(grid_pos + ivec3(0, 0, -1))].type == FLUID) {
        // check solid
        if (grid_pos.z == 0 || grid_pos.z == grid_dim.z - 1) {
            cell[index].vel.z = 0;
        } else {
            cell[index].vel.z -= scale * (cell[index].pressure - cell[get_grid_index(grid_pos + ivec3(0, 0, -1))].pressure);
        }
    } else {
        cell[index].vel_unknown = 1;
    }

    // hack to tempfix bug for demo
    if (grid_pos.x == grid_dim.x - 1) {
        cell[index].vel.x = cell[get_grid_index(grid_pos + ivec3(-1, 0, 0))].vel.x;
    }
    if (grid_pos.y == grid_dim.y - 1) {
        cell[index].vel.y = cell[get_grid_index(grid_pos + ivec3(0, -1, 0))].vel.y;
    }
    if (grid_pos.z == grid_dim.z - 1) {
        cell[index].vel.z = cell[get_grid_index(grid_pos + ivec3(0, 0, -1))].vel.z;
    }
}
