flat in int gs_discard;
in vec3 gs_color;
out vec4 frag_color;

void main() {
    if (gs_discard == 1)
        discard;
    frag_color = vec4(gs_color, 1);
}
