void build_a() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);
    uint index = get_grid_index(grid_pos);

    cell[index].pressure = 0;
    // warm start - don't clear guess

}
