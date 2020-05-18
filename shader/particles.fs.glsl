in vec4 color;
in vec3 vs_particle_pos;
in float vs_particle_radius;
out vec4 frag_color;

uniform vec3 look;
uniform vec4 viewport;
uniform mat4 projection;
uniform mat4 view;

const bool shaded = true;

// adapted from https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
vec3 ray_sphere_normal(vec3 r0, vec3 rd, vec3 s0, float sr) {
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
        return vec3(0);
    }
    vec3 isect = r0 + rd * (-b + sqrt((b*b) - 4.0*a*c))/(2.0*a);
    return normalize(isect - s0);
}

// https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space#From_gl_FragCoord
vec3 eye_pos() {
    vec4 ndcPos;
    ndcPos.xy = ((2.0 * gl_FragCoord.xy) - (2.0 * viewport.xy)) / (viewport.zw) - 1;
    ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) /
        (gl_DepthRange.far - gl_DepthRange.near);
    ndcPos.w = 1.0;

    vec4 clipPos = ndcPos / gl_FragCoord.w;
    return (inverse(projection * view) * clipPos).xyz;
}

void main() {
    if (shaded) {
        vec3 eye = eye_pos();
        vec3 normal = ray_sphere_normal(eye, look, vs_particle_pos, vs_particle_radius); 
        frag_color = vec4(shade(vs_particle_pos, normalize(look), normal, color.rgb * 0.3, color.rgb * 0.5, vec3(0.3), 16), color.a);
        // frag_color = vec4(normal, 1.0);
    }
    else {
        frag_color = color;
    }
}
