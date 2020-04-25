layout (location=0) in vec3 pos;
layout (location=1) in vec3 vel;
layout (location=2) in int marker;
out vec4 vs_color;

uniform mat4 projection;
uniform mat4 view;
const int display_mode=1;

void main() {
    gl_Position = projection * view * vec4(pos, 1.0);    

    if (display_mode == 0) {
        if (marker == AIR) {vs_color = vec4(0.5, 0.5, 0.5, 0.5);}
        else if (marker == SOLID) {vs_color = vec4(0.0, 1.0, 0.0, 1.0);}
        else if (marker == FLUID) {vs_color = vec4(0.0, 0.0, 1.0, 1.0);}
        else {vs_color = vec4(1.0, 0, 0, 0.5);}
    }
    if (display_mode == 1) {
        vs_color = vec4(vel / 0.001 * 0.5 + 0.5, 1.0);
    }
}
