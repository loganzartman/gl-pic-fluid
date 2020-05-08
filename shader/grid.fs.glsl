in vec3 vs_color;
flat in int discard_fragment;
out vec4 frag_color;

void main() {
    if (discard_fragment == 1) {
        discard;
    } else {
        frag_color = vec4(vs_color, 1);
    }
}
