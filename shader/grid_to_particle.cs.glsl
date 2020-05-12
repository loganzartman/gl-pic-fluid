layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

vec3 interpolate_velocity(uint index, ivec3 dimension_offset) {
    // interpolates velocity from 8 nearby grid corners
    // dimension_offset should correspond to the component of velocity being interpolated
    // other components of interpolated velocity are not meaningful

    // u
    // this part will change for each u, v, w
    ivec3 base_coord = get_grid_coord(particle[index].pos, -dimension_offset);
    vec3 weights = (particle[index].pos - get_world_coord(base_coord, dimension_offset)) / cell_size;

    // trilinearly interpolate 8 nearby grid velocity values to particle
    // x interpolation (gets values from all 8 grid corners)
    vec3 vel_x1 = cell[get_grid_index(base_coord + ivec3(0, 0, 0))].vel * (1 - weights.x) 
        + cell[get_grid_index(base_coord + ivec3(1, 0, 0))].vel * weights.x;
    vec3 vel_x2 = cell[get_grid_index(base_coord + ivec3(0, 1, 0))].vel * (1 - weights.x) 
        + cell[get_grid_index(base_coord + ivec3(1, 1, 0))].vel * weights.x;
    vec3 vel_x3 = cell[get_grid_index(base_coord + ivec3(0, 1, 1))].vel * (1 - weights.x) 
        + cell[get_grid_index(base_coord + ivec3(1, 1, 1))].vel * weights.x;
    vec3 vel_x4 = cell[get_grid_index(base_coord + ivec3(0, 0, 1))].vel * (1 - weights.x) 
        + cell[get_grid_index(base_coord + ivec3(1, 0, 1))].vel * weights.x;
    
    // y interpolation
    vec3 vel_y1 = vel_x1 * (1 - weights.y) + vel_x2 * weights.y;
    vec3 vel_y2 = vel_x4 * (1 - weights.y) + vel_x3 * weights.y;

    // z interpolation
    vec3 vel = vel_y1 * (1 - weights.z) + vel_y2 * weights.z;
    return vel;
}

void main() {
    vec3 grid_size = bounds_max - bounds_min;
    uint index = gl_WorkGroupID.x;

    // clear particle velocity
    particle[index].vel = vec3(0);

    particle[index].vel.x = interpolate_velocity(index, ivec3(1, 0, 0)).x;
    particle[index].vel.y = interpolate_velocity(index, ivec3(0, 1, 0)).y;
    particle[index].vel.z = interpolate_velocity(index, ivec3(0, 0, 1)).z;
    
    // debug
    // particle[index].color = vec4(weights, 1.0); // looks good
    // particle[index].color = vec4(base_coord, 1.0); // looks good...
    particle[index].color = vec4(particle[index].vel, 1.0);
}

