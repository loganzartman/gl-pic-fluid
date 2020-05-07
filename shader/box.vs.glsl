layout (location=0) in vec3 pos;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = vec4(pos, 1.0);
}
