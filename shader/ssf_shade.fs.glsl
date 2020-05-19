layout(binding=0) uniform sampler2D color_tex;
layout(binding=1) uniform sampler2D depth_tex;
layout(binding=2) uniform sampler2D sphere_pos_tex;

uniform ivec2 resolution;
uniform mat4 projection;
uniform mat4 inv_view;
uniform vec3 eye;
uniform vec3 look;

out vec4 frag_color;

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(resolution);
    vec4 color = texture(color_tex, uv);
    float depth = texture(depth_tex, uv).x;

    // compute normals with finite differences
    // sample eye-space positions of current pixel and neighbors
    vec4 eye_pos = texture(sphere_pos_tex, uv);
    vec4 eye_pos_px = texture(sphere_pos_tex, uv + vec2(1.0 / resolution.x, 0));
    vec4 eye_pos_py = texture(sphere_pos_tex, uv + vec2(0, -1.0 / resolution.y));
    vec4 eye_pos_nx = texture(sphere_pos_tex, uv + vec2(1.0 / resolution.x, 0));
    vec4 eye_pos_ny = texture(sphere_pos_tex, uv + vec2(0, -1.0 / resolution.y));

    // choose smaller difference to handle edges
    vec3 ab, ac;
    if (abs(eye_pos_px.z - eye_pos.z) < abs(eye_pos.z - eye_pos_nx.z)) {
        ab = eye_pos_px.xyz - eye_pos.xyz;
    } else {
        ab = eye_pos.xyz - eye_pos_nx.xyz;
    }
    if (abs(eye_pos_py.z - eye_pos.z) < abs(eye_pos.z - eye_pos_ny.z)) {
        ac = eye_pos_py.xyz - eye_pos.xyz;
    } else {
        ac = eye_pos.xyz - eye_pos_py.xyz;
    }

    // compute normal
    vec3 eye_normal = normalize(cross(ab, ac));
    vec3 normal = (inv_view * vec4(eye_normal, 0)).xyz;

    vec3 phong_color = shade(eye, normalize(look), normal, vec3(0.1), vec3(0.5), vec3(0.1), 64);
    frag_color = vec4(normal, color.a);
    gl_FragDepth = depth;
}