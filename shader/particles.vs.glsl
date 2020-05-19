layout(location=0) in vec2 circle_offset;
layout(location=1) in vec3 particle_pos;
layout(location=2) in vec3 vel;
layout(location=3) in vec4 particle_color;
out vec4 color;
out vec3 vs_particle_pos;
out float vs_particle_radius;

uniform mat4 projection;
uniform mat4 view;

const int display_mode = 0;

void main() {
    float radius = 0.02;
    vs_particle_pos = particle_pos;
    vs_particle_radius = radius;

    vec3 particle_pos_view = (view * vec4(particle_pos, 1.0)).xyz;
    vec3 vertex_pos_view = particle_pos_view + vec3(circle_offset, 0) * radius;
    gl_Position = projection * vec4(vertex_pos_view, 1.0);
    if (display_mode == 0) {
        color = particle_color;
    }
    if (display_mode == 1) {
        color = vec4(vel * 0.5 + 0.5, 1.0);
    }
}
