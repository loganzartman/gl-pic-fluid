layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform float dt;
uniform vec3 look;
uniform vec3 eye;
uniform vec3 mouse_pos;
uniform vec3 mouse_vel;

const float mouse_range = 0.25;

bool ray_sphere_isect(vec3 r0, vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return false;
    }
    return true;
}

void main() {
    uint index = gl_WorkGroupID.x;
    // TODO: don't use explicit Euler integration
    particle[index].pos += particle[index].vel * dt;
    
    // jitter particle positions to prevent squishing
    const float jitter = 0.005;
    particle[index].pos += hash3(floatBitsToInt(particle[index].pos)) * jitter - 0.5 * jitter;

    vec3 epsilon = vec3(0.00001);//cell_size - 0.01;
    particle[index].pos = clamp(particle[index].pos, bounds_min + epsilon, bounds_max - epsilon);

    bool hit = ray_sphere_isect(mouse_pos, normalize(mouse_pos - eye), particle[index].pos, mouse_range);
    if (hit)
        particle[index].vel += mouse_vel;
}
