#version 450

layout (set = 1, binding = 0) uniform ModelInfo {
    mat4 Model;
    vec4 FillColor;
} modelInfo;

layout (set = 2, binding = 0) uniform sampler2D imageSampler;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;


void main() {
    vec4 fillColor = modelInfo.FillColor;
    vec4 color = texture(imageSampler, in_uv);

    out_color = color * fillColor;
}