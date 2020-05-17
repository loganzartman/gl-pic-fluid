layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    cell[index].vel = vec3(
        fix2float(p2g_transfer[index].u), 
        fix2float(p2g_transfer[index].v), 
        fix2float(p2g_transfer[index].w)
    );
}
