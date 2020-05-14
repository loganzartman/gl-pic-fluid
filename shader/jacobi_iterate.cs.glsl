layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    float LUp = 0;
    
    if (grid_pos.x > 0) {
        uint i = get_grid_index(grid_pos + ivec3(-1, 0, 0));
        LUp -= cell[i].pressure * cell[i].a_x;
    }
    if (grid_pos.y > 0) {
        uint i = get_grid_index(grid_pos + ivec3(0, -1, 0));
        LUp -= cell[i].pressure * cell[i].a_y;
    }
    if (grid_pos.z > 0) {
        uint i = get_grid_index(grid_pos + ivec3(0, 0, -1));
        LUp -= cell[i].pressure * cell[i].a_z;
    }
    if (grid_pos.x < grid_dim.x - 1) {
        uint i = get_grid_index(grid_pos + ivec3(1, 0, 0));
        LUp += cell[i].pressure * cell[i].a_x;
    }
    if (grid_pos.y < grid_dim.y - 1) {
        uint i = get_grid_index(grid_pos + ivec3(0, 1, 0));
        LUp += cell[i].pressure * cell[i].a_y;
    }
    if (grid_pos.z < grid_dim.z - 1) {
        uint i = get_grid_index(grid_pos + ivec3(0, 0, 1));
        LUp += cell[i].pressure * cell[i].a_z;
    }

    cell[index].pressure = 1.0 / cell[index].a_diag * (cell[index].rhs - LUp * cell[index].pressure_guess);
}
