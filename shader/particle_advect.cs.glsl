layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;

void main() {
    uint index = gl_WorkGroupID.x;
    // TODO: don't use explicit Euler integration
    particle[index].pos += particle[index].vel * dt;
    particle[index].pos = clamp(particle[index].pos, bounds_min, bounds_max);
}
