#version 450

layout (set = 0, binding = 0) uniform RenderInfo {
    mat4 Projection;
} renderInfo;

layout (set = 1, binding = 0) uniform ModelInfo {
    mat4 Transform;
    vec4 FillColor;
    vec4 StrokeColor;
    vec2 Scale;
    vec2 Radius;
    float StrokeWidth;
} modelInfo;

layout(location = 0) out vec2 out_RectPos;


vec2 positions[6] = vec2[](
vec2(0.0, 0.0),
vec2(0.0, 1.0),
vec2(1.0, 0.0),
vec2(1.0, 0.0),
vec2(0.0, 1.0),
vec2(1.0, 1.0)
);


void main() {
    vec2 vertexPos = positions[gl_VertexIndex];
    out_RectPos = vertexPos * modelInfo.Scale;

    gl_Position = renderInfo.Projection * modelInfo.Transform * vec4(vertexPos, 0.0f, 1.0f);
}