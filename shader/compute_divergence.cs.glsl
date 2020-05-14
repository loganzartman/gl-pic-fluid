layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    cell[index].rhs = 0;

    if (cell[index].type != FLUID) {
        return;
    }

    if (grid_pos.x < grid_dim.x - 1) {
        uint index1 = get_grid_index(grid_pos + ivec3(1, 0, 0));
        cell[index].rhs -= (cell[index1].vel.x - cell[index].vel.x) / cell_size.x; 
    }
    if (grid_pos.y < grid_dim.y - 1) {
        uint index1 = get_grid_index(grid_pos + ivec3(0, 1, 0));
        cell[index].rhs -= (cell[index1].vel.y - cell[index].vel.y) / cell_size.y; 
    }
    if (grid_pos.z < grid_dim.z - 1) {
        uint index1 = get_grid_index(grid_pos + ivec3(0, 0, 1));
        cell[index].rhs -= (cell[index1].vel.z - cell[index].vel.z) / cell_size.z; 
    }
}
