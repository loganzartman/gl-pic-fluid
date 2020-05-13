layout (points) in;
layout (line_strip, max_vertices=2) out;

in vec3 vs_a[];
in vec3 vs_b[];
in vec4 vs_color[];
out vec4 gs_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gs_color = vs_color[0];

    gl_Position = projection * view * vec4(vs_a[0], 1.0);
    EmitVertex();

    gl_Position = projection * view * vec4(vs_b[0], 1.0);
    EmitVertex();
    
    EndPrimitive();
}
