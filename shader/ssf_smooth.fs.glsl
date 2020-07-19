layout(binding=0) uniform sampler2D sphere_pos_tex;

uniform ivec2 resolution;
uniform mat4 projection;

in vec2 uv;
layout(location=1) out vec4 sphere_pos;

float resolution_factor = resolution.y / 300; 

// based on https://www.shadertoy.com/view/4dfGDH
#define ITERS 32
#define KERNEL_SIZE 32.0
#define SIGMA 32.0
#define BSIGMA 0.3

// hash3 (c) 2017 Inigo Quilez
const uint krand = 1103515245U;  // GLIB C
vec3 hash3(uvec3 x) {
    x = ((x>>8U)^x.yzx)*krand;
    x = ((x>>8U)^x.yzx)*krand;
    x = ((x>>8U)^x.yzx)*krand;
    return vec3(x)*(1.0/float(0xffffffffU));
}


float normpdf(in float x, in float sigma) {
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in vec3 v, in float sigma) {
	return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

void main() {
    vec4 src_sphere_pos = texelFetch(sphere_pos_tex, ivec2(gl_FragCoord.xy), 0);
    float depth = src_sphere_pos.z;
    
    float Z = 0;
    float final_depth = 0;
    float bZ = 1.0 / normpdf(0.0, BSIGMA);
    for (int i = 0; i < ITERS; ++i) {
        // random position on disc
        vec2 rand = hash3(uvec3(gl_FragCoord.xy * i, -i)).xy;
        float disp_angle = rand.x * 6.28;
        float disp_len = rand.y * KERNEL_SIZE;
        vec2 displacement = vec2(
            cos(disp_angle) * disp_len, 
            sin(disp_angle) * disp_len
        );

        float f = normpdf(disp_len, SIGMA);

        // bilateral filter
        float cur_depth = texelFetch(sphere_pos_tex, ivec2(floor(gl_FragCoord.xy + displacement)), 0).z;
        float factor = normpdf(cur_depth - depth, BSIGMA) * bZ * f;
        Z += factor;
        final_depth += factor * cur_depth;
    }

    sphere_pos = vec4(src_sphere_pos.xy, final_depth / Z, src_sphere_pos.w);
}
