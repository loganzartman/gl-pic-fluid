layout (location=0) in vec2 vert;

out vec2 uv;

void main() {
    gl_Position = vec4(vert * 2. - 1., 0, 1);
    uv = vert;
}
