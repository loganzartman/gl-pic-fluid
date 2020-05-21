layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    if (cell[index].type == AIR) {
        cell[index].pressure = 0;
        return;
    }
    if (cell[index].type == SOLID) {
        // invalid
        cell[index].pressure = 0;
        return;
    }

    float L_Up = 0;

    if (grid_pos.x > 0) {
        uint j = get_grid_index(grid_pos + ivec3(-1, 0, 0));
        L_Up += cell[j].a_x * cell[j].pressure_guess;
    }
    if (grid_pos.y > 0) {
        uint j = get_grid_index(grid_pos + ivec3(0, -1, 0));
        L_Up += cell[j].a_y * cell[j].pressure_guess;
    }
    if (grid_pos.z > 0) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, -1));
        L_Up += cell[j].a_z * cell[j].pressure_guess;
    }

    if (grid_pos.x < grid_dim.x - 2) {
        uint j = get_grid_index(grid_pos + ivec3(1, 0, 0));
        L_Up += cell[index].a_x * cell[j].pressure_guess;
    }
    if (grid_pos.y < grid_dim.y - 2) {
        uint j = get_grid_index(grid_pos + ivec3(0, 1, 0));
        L_Up += cell[index].a_y * cell[j].pressure_guess;
    }
    if (grid_pos.z < grid_dim.z - 2) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, 1));
        L_Up += cell[index].a_z * cell[j].pressure_guess;
    }

    if (cell[index].a_diag != 0)
        cell[index].pressure = 1.0 / cell[index].a_diag * (cell[index].rhs - L_Up);
}
