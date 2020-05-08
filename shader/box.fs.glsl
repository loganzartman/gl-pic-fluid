in vec3 gs_vertex_pos;
flat in vec3 normal;
out vec4 frag_color;

uniform vec3 eye;
uniform mat4 projection;
uniform mat4 view;

void main() {
    vec3 look = normalize(gs_vertex_pos - eye);
    vec3 ka = vec3(0.5, 0.5, 0.5);
    vec3 kd = vec3(0.3, 0.3, 0.3);
    vec3 ks = vec3(0.3, 0.3, 0.3);
    frag_color = vec4(shade(gs_vertex_pos, look, normal, ka, kd, ks, 32), 0.7);
}
