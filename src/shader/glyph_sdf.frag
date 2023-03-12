#version 450

layout (set = 1, binding = 0) uniform ModelInfo {
    mat4 Model;
    vec4 FillColor;
} modelInfo;

layout (set = 2, binding = 0) uniform sampler2D glyphSampler;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

#define epsilon float(1e-37)

#define scalar (128.0f / 255.0f)

void main() {
    vec4 fill = modelInfo.FillColor;
    float d = texture(glyphSampler, in_uv).r;
    float aaf = fwidth(d) / 2;

    float alpha = smoothstep(0.5f - aaf, 0.5f + aaf, d);
    //float alpha = step(0.5f, d);

    fill.a *= alpha;

    out_color = fill;
}