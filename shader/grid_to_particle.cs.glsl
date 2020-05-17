layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float pic_flip_blend;

vec3 lerp_vel(uint index, ivec3 component) {
    // interpolates velocity from 8 nearby grid corners
    // dimension_offset should correspond to the component of velocity being interpolated
    // other components of interpolated velocity are not meaningful

    // this part will change for each u, v, w
    ivec3 dimension_offset = ivec3(1) - component;
    ivec3 base_coord = get_grid_coord(particle[index].pos, -dimension_offset);
    vec3 weights = (particle[index].pos - get_world_coord(base_coord, dimension_offset)) / cell_size;

    // trilinearly interpolate 8 nearby grid velocity values to particle
    // x interpolation (gets values from all 8 grid corners)
    vec3 vel_x1 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 0, 0)))].vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 0, 0)))].vel * weights.x;
    vec3 vel_x2 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 1, 0)))].vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 1, 0)))].vel * weights.x;
    vec3 vel_x3 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 0, 1)))].vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 0, 1)))].vel * weights.x;
    vec3 vel_x4 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 1, 1)))].vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 1, 1)))].vel * weights.x;
    
    // y interpolation
    vec3 vel_y1 = vel_x1 * (1 - weights.y) + vel_x2 * weights.y;
    vec3 vel_y2 = vel_x3 * (1 - weights.y) + vel_x4 * weights.y;

    // z interpolation
    vec3 vel = vel_y1 * (1 - weights.z) + vel_y2 * weights.z;
    return vel;
}

vec3 lerp_old_vel(uint index, ivec3 component) {
    // interpolates delta velocity from 8 nearby grid corners
    // dimension_offset should correspond to the component of velocity being interpolated
    // other components of interpolated velocity are not meaningful

    ivec3 dimension_offset = ivec3(1) - component;
    ivec3 base_coord = get_grid_coord(particle[index].pos, -dimension_offset);
    vec3 weights = (particle[index].pos - get_world_coord(base_coord, dimension_offset)) / cell_size;

    // trilinearly interpolate 8 nearby grid velocity values to particle
    // x interpolation (gets values from all 8 grid corners)
    vec3 vel_x1 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 0, 0)))].old_vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 0, 0)))].old_vel * weights.x;
    vec3 vel_x2 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 1, 0)))].old_vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 1, 0)))].old_vel * weights.x;
    vec3 vel_x3 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 0, 1)))].old_vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 0, 1)))].old_vel * weights.x;
    vec3 vel_x4 = cell[get_grid_index(offset_clamped(base_coord, ivec3(0, 1, 1)))].old_vel * (1 - weights.x) 
                + cell[get_grid_index(offset_clamped(base_coord, ivec3(1, 1, 1)))].old_vel * weights.x;
    
    // y interpolation
    vec3 vel_y1 = vel_x1 * (1 - weights.y) + vel_x2 * weights.y;
    vec3 vel_y2 = vel_x3 * (1 - weights.y) + vel_x4 * weights.y;

    // z interpolation
    vec3 vel = vel_y1 * (1 - weights.z) + vel_y2 * weights.z;
    return vel;
}

void main() {
    vec3 grid_size = bounds_max - bounds_min;
    uint index = gl_WorkGroupID.x;

    float u = lerp_vel(index, ivec3(1, 0, 0)).x;
    float v = lerp_vel(index, ivec3(0, 1, 0)).y;
    float w = lerp_vel(index, ivec3(0, 0, 1)).z;
    float ou = lerp_old_vel(index, ivec3(1, 0, 0)).x;
    float ov = lerp_old_vel(index, ivec3(0, 1, 0)).y;
    float ow = lerp_old_vel(index, ivec3(0, 0, 1)).z;

    float flipu = particle[index].vel.x + u - ou;
    float flipv = particle[index].vel.y + v - ov;
    float flipw = particle[index].vel.z + w - ow;

    particle[index].vel.x = u * (1 - pic_flip_blend) + flipu * pic_flip_blend;
    particle[index].vel.y = v * (1 - pic_flip_blend) + flipv * pic_flip_blend;
    particle[index].vel.z = w * (1 - pic_flip_blend) + flipw * pic_flip_blend;
}
