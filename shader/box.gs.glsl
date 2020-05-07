layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

out vec3 gs_vertex_pos;
flat out vec3 normal; 

uniform mat4 projection;
uniform mat4 view;

void main() {
    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 bc = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    normal = normalize(cross(ab, bc));

    for (int n = 0; n < gl_in.length(); n++) {
        gl_Position = projection * view * gl_in[n].gl_Position;
        gs_vertex_pos = (gl_in[n].gl_Position).xyz;
        EmitVertex();
    }
    EndPrimitive();
}
