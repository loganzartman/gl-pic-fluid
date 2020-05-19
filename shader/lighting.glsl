vec3 phong_shade(vec3 light_direction, vec3 look, vec3 normal, vec3 ka, vec3 kd, vec3 ks, float shininess) {
    vec3 R = reflect(light_direction, normal);
    float diffuse = max(0, dot(light_direction, normal));
    float specular = pow(max(0, dot(R, look)), shininess);
    return ka + kd * diffuse + ks * specular;
}

vec3 shade(vec3 pos, vec3 look, vec3 normal, vec3 ka, vec3 kd, vec3 ks, float shininess) {
    return ka + phong_shade(normalize(vec3(-0.5, 0.8, 0) - pos), look, normal, vec3(0), kd, ks, shininess) +
                phong_shade(normalize(vec3(0.5, 0.8, 0) - pos), look, normal, vec3(0), kd, ks, shininess);
}
