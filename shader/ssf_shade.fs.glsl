layout(binding=0) uniform sampler2D color_tex;
layout(binding=1) uniform sampler2D depth_tex;
layout(binding=2) uniform sampler2D sphere_pos_tex;

uniform ivec2 resolution;
uniform mat4 projection;
uniform mat4 inv_view;
uniform vec3 eye;
uniform vec3 look;

out vec4 frag_color;

const vec3 k_absorption = vec3(0.3, 0.12, 0.05);
const vec3 k_reflection = vec3(1);

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(resolution);
    float thickness = texture(color_tex, uv).r;
    float depth = texture(depth_tex, uv).x;

    // compute normals with finite differences
    // sample eye-space positions of current pixel and neighbors
    vec4 eye_pos = texture(sphere_pos_tex, uv);
    vec4 eye_pos_px = texture(sphere_pos_tex, uv + vec2(1.0 / resolution.x, 0));
    vec4 eye_pos_py = texture(sphere_pos_tex, uv + vec2(0, 1.0 / resolution.y));
    vec4 eye_pos_nx = texture(sphere_pos_tex, uv + vec2(-1.0 / resolution.x, 0));
    vec4 eye_pos_ny = texture(sphere_pos_tex, uv + vec2(0, -1.0 / resolution.y));

    // choose smaller difference to handle edges
    vec3 ab, ac;
    if (abs(eye_pos_px.z - eye_pos.z) < abs(eye_pos_nx.z - eye_pos.z)) {
        ab = eye_pos_px.xyz - eye_pos.xyz;
    } else {
        ab = eye_pos.xyz - eye_pos_nx.xyz;
    }
    if (abs(eye_pos_py.z - eye_pos.z) < abs(eye_pos_ny.z - eye_pos.z)) {
        ac = eye_pos_py.xyz - eye_pos.xyz;
    } else {
        ac = eye_pos.xyz - eye_pos_ny.xyz;
    }

    // compute normal
    vec3 eye_normal = cross(ab, ac);
    vec3 normal = normalize((inv_view * vec4(eye_normal, 0)).xyz);

    vec3 world_pos = (inv_view * eye_pos).xyz;

    // beer's law (light absorption)
    vec3 transmitted_color = exp(-5 * k_absorption * thickness);

    // fresnel's law approximation
    // http://developer.download.nvidia.com/CgTutorial/cg_tutorial_chapter07.html
    const float fresnel_bias = 0;
    const float fresnel_scale = 1;
    const float fresnel_power = 3;
    float r = max(0, min(1, fresnel_bias + fresnel_scale * pow(1 + dot(-normalize(look), normal), fresnel_power)));

    vec3 reflected_color = k_reflection;
    vec3 fluid_color = r * reflected_color + (1 - r) * transmitted_color;

    frag_color = vec4(fluid_color, thickness);
    gl_FragDepth = depth;
}
