in vec4 color;
in vec3 vs_particle_pos;
in float vs_particle_radius;

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 sphere_pos;

uniform vec3 look;
uniform vec4 viewport;
uniform mat4 projection;
uniform mat4 view;
uniform int pass;

// adapted from https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
float ray_sphere_dist(vec3 r0, vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return -1.0;
    }
    float d0 = (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
    float d1 = (-b + sqrt((b*b) - 4.0*a*c))/(2.0*a);
    float dist = max(d0, d1);
    return dist;
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
    frag_color = vec4(1,0,0,1);
    vec3 eye = eye_pos();
    
    float dist = ray_sphere_dist(eye, look, vs_particle_pos, vs_particle_radius);
    if (dist < 0) {
        discard;
    }

    vec3 isect = eye + look * dist;
    vec3 normal = normalize(isect - vs_particle_pos);
    
    // compute fragment depth on sphere in eye space and NDC
    vec4 eye_isect = view * vec4(isect, 1);
    vec4 ndc_isect = projection * eye_isect;

    if (pass == 0) {
        // thickness pass
        float f = (eye_isect.z - (view * vec4(vs_particle_pos, 1)).z) / vs_particle_radius;
        frag_color = vec4(vec3(f * 0.02), 1);
    }
    if (pass == 1) {
        // position pass
        sphere_pos = eye_isect;
        gl_FragDepth = ndc_isect.z / ndc_isect.w; // clip-space depth
    }
}
