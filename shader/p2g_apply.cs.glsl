layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    if (p2g_transfer[index].is_fluid)
        cell[index].type = FLUID;

    if (p2g_transfer[index].weight_u != 0)
        cell[index].vel.x = getAtomicFloat(p2g_transfer[index].u) / getAtomicFloat(p2g_transfer[index].weight_u);
    if (p2g_transfer[index].weight_v != 0)
        cell[index].vel.y = getAtomicFloat(p2g_transfer[index].v) / getAtomicFloat(p2g_transfer[index].weight_v);
    if (p2g_transfer[index].weight_w != 0)
        cell[index].vel.z = getAtomicFloat(p2g_transfer[index].w) / getAtomicFloat(p2g_transfer[index].weight_w);

    p2g_transfer[index].u = 0;
    p2g_transfer[index].v = 0;
    p2g_transfer[index].w = 0;
    p2g_transfer[index].weight_u = 0;
    p2g_transfer[index].weight_v = 0;
    p2g_transfer[index].weight_w = 0;
    p2g_transfer[index].is_fluid = false;
}
