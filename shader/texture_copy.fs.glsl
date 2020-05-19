layout(binding=0) uniform sampler2D color_tex;
layout(binding=1) uniform sampler2D depth_tex;

in vec2 uv;
layout(location=0) out vec4 frag_color;

void main() {
    frag_color = texture(color_tex, uv);
    gl_FragDepth = texture(depth_tex, uv).r;
}
