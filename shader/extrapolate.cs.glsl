layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    if (cell[index].vel_unknown == 0) {
        return;
    }

    vec3 sum = vec3(0);
    int count = 0;
    if (grid_pos.x > 0) {
        uint j = get_grid_index(grid_pos + ivec3(-1, 0, 0));
        if (cell[j].vel_unknown < 1) {
            sum += cell[j].vel;
            count++;
        }
    }
    if (grid_pos.y > 0) {
        uint j = get_grid_index(grid_pos + ivec3(0, -1, 0));
        if (cell[j].vel_unknown < 1) {
            sum += cell[j].vel;
            count++;
        }
    }
    if (grid_pos.z > 0) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, -1));
        if (cell[j].vel_unknown < 1) {
            sum += cell[j].vel;
            count++;
        }
    }
    if (grid_pos.x < grid_dim.x - 1) {
        uint j = get_grid_index(grid_pos + ivec3(1, 0, 0));
        if (cell[j].vel_unknown < 1) {
            sum += cell[j].vel;
            count++;
        }
    }
    if (grid_pos.y < grid_dim.y - 1) {
        uint j = get_grid_index(grid_pos + ivec3(0, 1, 0));
        if (cell[j].vel_unknown < 1) {
            sum += cell[j].vel;
            count++;
        }
    }
    if (grid_pos.z < grid_dim.z - 1) {
        uint j = get_grid_index(grid_pos + ivec3(0, 0, 1));
        if (cell[j].vel_unknown < 1) {
            sum += cell[j].vel;
            count++;
        }
    }
    
    if (count > 0) {
        cell[index].vel = sum / count;
        cell[index].vel_unknown = 2;
    }
}
