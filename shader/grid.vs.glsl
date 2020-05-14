layout (location=0) in vec3 pos;
layout (location=1) in vec3 vel;
layout (location=2) in int type;
layout (location=3) in float rhs;
out vec3 vs_color;
flat out int discard_fragment;

uniform mat4 projection;
uniform mat4 view;
const int display_mode=2;

void main() {
    gl_Position = projection * view * vec4(pos, 1.0);    

    vs_color = vec3(1.0, 0, 1.0);
    discard_fragment = 0;
    // if (type == AIR) {discard_fragment = 1;}

    if (display_mode == 0) {
        if (type == SOLID) {vs_color = vec3(0.0, 1.0, 0.0);}
        if (type == FLUID) {vs_color = vec3(0.0, 0.0, 1.0);}
    }
    if (display_mode == 1) {
        vs_color = vec3(vel);
    }
    if (display_mode == 2) {
        vs_color = vec3(abs(rhs));
    }
}
