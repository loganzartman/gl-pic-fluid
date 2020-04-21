layout (location=0) in vec3 pos;
out vec2 uv;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(pos, 1.0);
    uv = gl_Position.xy;
}
