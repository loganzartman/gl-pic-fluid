in vec3 gs_vertex_pos;
flat in vec3 normal;
out vec4 frag_color;

uniform vec3 eye;
uniform mat4 projection;
uniform mat4 view;

const float shininess = 32;

void main() {
    vec3 light_direction = normalize((inverse(projection * view) * vec4(0, 0, -1, 0)).xyz);
    vec3 look = normalize(gs_vertex_pos - eye);
    vec3 R = reflect(-light_direction, normal);

    float diffuse = max(0, dot(light_direction, normal));
    float specular = pow(max(0, dot(R, -look)), shininess);

    vec3 ka = vec3(0.5, 0.5, 0.5);
    vec3 kd = vec3(0.3, 0.3, 0.3);
    vec3 ks = vec3(0.3, 0.3, 0.3);
    frag_color = vec4(ka + kd * diffuse + ks * specular, 0.7);
}
