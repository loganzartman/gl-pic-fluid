layout(location=0) in vec2 circle_offset;
layout(location=1) in vec3 particle_pos;
layout(location=2) in vec4 particle_color;
out vec4 color;
uniform mat4 projection;
uniform mat4 view;

void main() {
    const float radius = 0.01;
    vec3 particle_pos_world = (view * vec4(particle_pos, 1.0)).xyz;
    vec3 vertex_pos_world = particle_pos_world + vec3(circle_offset, 0) * radius * 0.5;
    gl_Position = projection * vec4(vertex_pos_world, 1.0);
    color = particle_color;
}
