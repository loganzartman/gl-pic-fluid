layout (location=0) in vec3 a;
layout (location=1) in vec3 b;
layout (location=2) in vec4 color;

out vec3 vs_a;
out vec3 vs_b;
out vec4 vs_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vs_a = a;
    vs_b = b;
    vs_color = color;
}
