layout (location=0) in vec3 pos;
layout (location=1) in vec3 v;
layout (location=2) in int marker;
out vec4 vs_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(pos, 1.0);    
    vs_color = vec4(marker, marker % 2, log(marker) / 10, 1.0);
}
