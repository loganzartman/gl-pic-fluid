layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

uniform int start_index;

void main() {
    uint index = start_index + gl_LocalInvocationIndex.x;

    if (index >= particle.length()) {
        return;
    }

    ivec3 base_coord = get_grid_coord(particle[index].pos, ivec3(0));
    uint grid_index = get_grid_index(base_coord);

    atomicAdd(p2g_transfer[grid_index].u, float2fix(particle[index].vel.x));
    atomicAdd(p2g_transfer[grid_index].v, float2fix(particle[index].vel.y));
    atomicAdd(p2g_transfer[grid_index].w, float2fix(particle[index].vel.z));
}
