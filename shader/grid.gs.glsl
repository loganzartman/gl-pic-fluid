layout (points) in;
layout (points, max_vertices=4) out;

flat in int vs_display_mode[];
flat in int vs_discard[];
in vec3 vs_pos[];
in vec3 vs_color[];
in vec3 vs_vel[];
flat out int gs_discard;
out vec3 gs_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    const float len = 0.1;
    uint id = gl_InvocationID;
    mat4 pv = projection * view;
    gs_discard = vs_discard[0];

    if (false && vs_display_mode[0] == 1) {
        // face velocities
        gs_color = vec3(vs_vel[0].x * 0.5 + 0.5, 0.5, 0.5);
        gl_Position = pv * vec4(vs_pos[0] + cell_size * vec3(0, 0.5, 0.5), 1);
        EmitVertex();
        gs_color = vec3(0.5, vs_vel[0].y * 0.5 + 0.5, 0.5);
        gl_Position = pv * vec4(vs_pos[0] + cell_size * vec3(0.5, 0, 0.5), 1);
        EmitVertex();
        gs_color = vec3(0.5, 0.5, vs_vel[0].z * 0.5 + 0.5);
        gl_Position = pv * vec4(vs_pos[0] + cell_size * vec3(0.5, 0.5, 0), 1);
        EmitVertex();
    } else {
        // central quantity viz
        gs_color = vs_color[0];
        gl_Position = pv * vec4(vs_pos[0] + cell_size * 0.5, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
