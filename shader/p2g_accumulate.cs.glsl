layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

uniform int start_index;

void scatter_part(ivec3 coord, vec3 weights, vec3 vel) {
    int index = get_grid_index(coord);
    float weight = weights.x * weights.y * weights.z;
    // TODO: set velocity unknown flag
    if (vel.x != 0) {
        atomicAddFloat(p2g_transfer[index].u, vel.x * weight);
        atomicAddFloat(p2g_transfer[index].weight_u, weight);
    }
    if (vel.y != 0) {
        atomicAddFloat(p2g_transfer[index].v, vel.y * weight);
        atomicAddFloat(p2g_transfer[index].weight_v, weight);
    }
    if (vel.z != 0) {
        atomicAddFloat(p2g_transfer[index].w, vel.z * weight);
        atomicAddFloat(p2g_transfer[index].weight_w, weight);
    }
}

void scatter_vel(uint index, ivec3 component) {
    ivec3 offset =  component;
    ivec3 base_coord = get_grid_coord(particle[index].pos, -offset);

    // interpolation weights
    vec3 wgt = (particle[index].pos - get_world_coord(base_coord, offset)) / cell_size;

    vec3 comp_vel = vec3(component) * particle[index].vel;
    scatter_part(offset_clamped(base_coord, ivec3(0, 0, 0)), vec3(wgt.x, wgt.y, wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(1, 0, 0)), vec3(1-wgt.x, wgt.y, wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(0, 1, 0)), vec3(wgt.x, 1-wgt.y, wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(0, 0, 1)), vec3(wgt.x, wgt.y, 1-wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(1, 1, 0)), vec3(1-wgt.x, 1-wgt.y, wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(0, 1, 1)), vec3(wgt.x, 1-wgt.y, 1-wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(1, 0, 1)), vec3(1-wgt.x, wgt.y, 1-wgt.z), comp_vel);
    scatter_part(offset_clamped(base_coord, ivec3(1, 1, 1)), vec3(1-wgt.x, 1-wgt.y, 1-wgt.z), comp_vel);
}

void main() {
    uint index = start_index + gl_LocalInvocationIndex.x;

    if (index >= particle.length()) {
        return;
    }

    ivec3 base_coord = get_grid_coord(particle[index].pos, ivec3(0));
    uint grid_index = get_grid_index(base_coord);
    p2g_transfer[grid_index].is_fluid = true; // does not need to be atomic

    scatter_vel(index, ivec3(1, 0, 0));
    scatter_vel(index, ivec3(0, 1, 0));
    scatter_vel(index, ivec3(0, 0, 1));
}
