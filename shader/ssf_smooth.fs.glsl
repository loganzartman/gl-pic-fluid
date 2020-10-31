layout(binding=0) uniform sampler2D sphere_pos_tex;

uniform ivec2 resolution;
uniform mat4 projection;

in vec2 uv;
layout(location=1) out vec4 sphere_pos;

float resolution_factor = resolution.y / 300; 

// based on https://www.shadertoy.com/view/4dfGDH
#define ITERS 32
#define KERNEL_SIZE 12.0
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

vec3 hashi3(uint n) {
    // integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    uvec3 k = n * uvec3(n,n*16807U,n*48271U);
    return vec3( k & uvec3(0x7fffffffU))/float(0x7fffffff);
}

vec2 getOffset(vec2 frag_coord, int i) {
    // vec3 rand = hash3(uvec3(frag_coord.xy, i));
    vec3 rand = hashi3(uint((frag_coord.y * resolution.x + frag_coord.x) * ITERS + i));
    return vec2(rand.x * rand.z, rand.y);
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
    
    float n = 0;
    float d = 0;

    for (int i = 0; i < ITERS; ++i) {
        vec2 offset = getOffset(gl_FragCoord.xy, i);
        float len = offset.x * KERNEL_SIZE;
        float angle = offset.y * 6.28;
        vec2 disp = vec2(cos(angle) * len, sin(angle) * len);

        float sample_depth = texelFetch(sphere_pos_tex, ivec2(floor(gl_FragCoord.xy + disp)), 0).z;
        float factor = normpdf(len, SIGMA);
        factor *= normpdf(sample_depth - depth, BSIGMA);
        n += sample_depth * factor;
        d += factor;
    }

    float final_depth = n / d;
    sphere_pos = vec4(src_sphere_pos.xy, final_depth, src_sphere_pos.w);
}
