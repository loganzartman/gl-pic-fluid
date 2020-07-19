layout(binding=0) uniform sampler2D color_tex;
layout(binding=1) uniform sampler2D depth_tex;
layout(binding=2) uniform sampler2D sphere_pos_tex;
layout(binding=3) uniform sampler2D scene_tex;

uniform ivec2 resolution;
uniform mat4 projection;
uniform mat4 inv_view;
uniform vec3 eye;
uniform vec3 look;

out vec4 frag_color;

const float base_translucency = 0.4;
const float refractivity = 0.05;
const vec3 k_absorption = vec3(0.9, 0.22, 0.05);
const vec4 k_reflection = vec4(1);

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(resolution);
    float thickness = 1 - pow(1 - texture(color_tex, uv).r, 10);
    float depth = texture(depth_tex, uv).x;

    // compute normals with finite differences
    // sample eye-space positions of current pixel and neighbors
    vec4 eye_pos = texture(sphere_pos_tex, uv);
    vec4 eye_pos_px = texture(sphere_pos_tex, uv + vec2(1.0 / resolution.x, 0));
    vec4 eye_pos_py = texture(sphere_pos_tex, uv + vec2(0, 1.0 / resolution.y));
    vec4 eye_pos_nx = texture(sphere_pos_tex, uv + vec2(-1.0 / resolution.x, 0));
    vec4 eye_pos_ny = texture(sphere_pos_tex, uv + vec2(0, -1.0 / resolution.y));

    vec3 ab1 = eye_pos_px.xyz - eye_pos.xyz;
    vec3 ab2 = eye_pos.xyz - eye_pos_nx.xyz;
    vec3 ab3 = eye_pos_px.xyz - eye_pos_nx.xyz;
    vec3 ac1 = eye_pos_py.xyz - eye_pos.xyz;
    vec3 ac2 = eye_pos.xyz - eye_pos_ny.xyz;
    vec3 ac3 = eye_pos_py.xyz - eye_pos_ny.xyz;
    vec3 ab = 0.33333 * (ab1 + ab2 + ab3);
    vec3 ac = 0.33333 * (ac1 + ac2 + ac3);

    // compute normal
    vec3 eye_normal = cross(ab, ac);
    vec3 normal = normalize((inv_view * vec4(eye_normal, 0)).xyz);

    vec3 world_pos = (inv_view * eye_pos).xyz;

    // fake refraction
    vec3 refracted_color = texture(scene_tex, uv + normal.xy * thickness * refractivity).rgb;

    // beer's law (light absorption)
    vec3 absorption_color = exp(-3 * k_absorption * thickness);
    float translucency = max(base_translucency, 1 - pow(thickness, 1));
    vec3 transmitted_color = (1 - translucency) * absorption_color + translucency * refracted_color;

    // fresnel's law approximation
    // http://developer.download.nvidia.com/CgTutorial/cg_tutorial_chapter07.html
    const float fresnel_bias = -0.1;
    const float fresnel_scale = 1;
    const float fresnel_power = 3;
    float r = max(0, min(1, fresnel_bias + fresnel_scale * pow(1 + dot(-normalize(look), normal), fresnel_power)));

    vec3 reflected_color = k_reflection.rgb;
    float rt_mix = r * k_reflection.a;
    frag_color = vec4(rt_mix * reflected_color + (1 - rt_mix) * transmitted_color, 1);
    // frag_color = vec4(normal * 0.5 + 0.5, 1.0);
    // frag_color = vec4(vec3(-eye_pos.z - 4.5) / 3, 1.0);

    gl_FragDepth = depth;
}
