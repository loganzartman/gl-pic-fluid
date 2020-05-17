layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;
uniform vec3 body_force;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    cell[index].old_vel = cell[index].vel;

    if (grid_pos.y < grid_dim.y - 1 && grid_pos.z < grid_dim.z - 1)
        cell[index].vel.x += body_force.x * dt;
    if (grid_pos.x < grid_dim.x - 1 && grid_pos.z < grid_dim.z - 1)
        cell[index].vel.y += body_force.y * dt;
    if (grid_pos.x < grid_dim.x - 1 && grid_pos.y < grid_dim.y - 1)
        cell[index].vel.z += body_force.z * dt;

    // wacky winds
    // cell[get_grid_index(grid_pos)].vel.y += sin(get_world_coord(grid_pos, ivec3(0)).x * 5) * 20 * dt;
    // cell[get_grid_index(grid_pos)].vel.x += cos(get_world_coord(grid_pos, ivec3(0)).y * 5) * 20 * dt;
    enforce_boundary_condition(grid_pos);
}
