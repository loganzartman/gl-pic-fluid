layout(binding=0) uniform sampler2D sphere_pos_tex;

uniform ivec2 resolution;
uniform mat4 projection;

in vec2 uv;
layout(location=1) out vec4 sphere_pos;

// https://www.shadertoy.com/view/4dfGDH
#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15
const float kernel[MSIZE] = float[MSIZE](0.031225216, 0.033322271, 0.035206333, 0.036826804, 0.038138565, 0.039104044, 0.039695028, 0.039894000, 0.039695028, 0.039104044, 0.038138565, 0.036826804, 0.035206333, 0.033322271, 0.031225216);

float normpdf(in float x, in float sigma) {
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in vec3 v, in float sigma) {
	return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

void main() {
    vec4 src_sphere_pos = texture(sphere_pos_tex, gl_FragCoord.xy / resolution);
    float depth = src_sphere_pos.z;
    
    const int k_size = (MSIZE - 1) / 2;

    float Z = 0;
    float final_depth = 0;
    float bZ = 1.0 / normpdf(0.0, BSIGMA);
    for (int i = -k_size; i <= k_size; ++i) {
        for (int j = -k_size; j <= k_size; ++j) {
            float cur_depth = texture(sphere_pos_tex, (gl_FragCoord.xy + vec2(i, j)) / resolution).z;
            float factor = normpdf(cur_depth - depth, BSIGMA) * bZ * kernel[k_size + j] * kernel[k_size + i];
            Z += factor;
            final_depth += factor * cur_depth;
        }
    }

    sphere_pos = vec4(src_sphere_pos.xy, final_depth / Z, src_sphere_pos.w);
}
