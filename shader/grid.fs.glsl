in vec3 gs_color;
out vec4 frag_color;

void main() {
    frag_color = vec4(gs_color, 1);
}
