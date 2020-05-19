in vec3 gs_vertex_pos;
flat in vec3 normal;
out vec4 frag_color;

uniform vec3 eye;
uniform mat4 projection;
uniform mat4 view;

const float eps = 0.0001;
const float check_scale = 3;
const vec3 kd_light = vec3(0.3);
const vec3 kd_dark = vec3(0.2);

void main() {
    vec3 p = gs_vertex_pos;
    bool light = fract((floor(p.x * check_scale + eps) + floor(p.y * check_scale + eps) + floor(p.z * check_scale + eps)) * 0.5) > 0.25;
    vec3 look = normalize(gs_vertex_pos - eye);
    vec3 ka = vec3(0.2);
    vec3 kd = light ? kd_light : kd_dark;
    vec3 ks = vec3(0.1);
    frag_color = vec4(shade(gs_vertex_pos, look, normal, ka, kd, ks, 32), 1.0);
}
