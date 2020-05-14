layout (location=0) in vec3 pos;
layout (location=1) in vec3 vel;
layout (location=2) in int type;
layout (location=3) in float rhs;
layout (location=4) in vec4 a;

flat out int vs_display_mode;
out vec3 vs_pos;
out vec3 vs_color;
out vec3 vs_vel;

uniform mat4 projection;
uniform mat4 view;
const int display_mode=2;

void main() {
    vs_display_mode = display_mode;
    vs_pos = pos;
    vs_vel = vel;
    vs_color = vec3(1.0, 0, 1.0);

    if (display_mode == 0) {
        if (type == SOLID) {vs_color = vec3(0.0, 1.0, 0.0);}
        if (type == FLUID) {vs_color = vec3(0.0, 0.0, 1.0);}
    }
    if (display_mode == 1) {
        vs_color = vec3(abs(vel));
    }
    if (display_mode == 2) {
        vs_color = vec3(abs(rhs));
    }
    if (display_mode == 3) {
        vs_color = vec3(abs(a.yzw));
    }
}
