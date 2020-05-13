layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;
uniform vec3 body_force;

void main() {
    ivec3 grid_pos = ivec3(gl_WorkGroupID);

    cell[get_grid_index(grid_pos)].vel += body_force * dt;
    enforce_boundary_condition(grid_pos);
}
