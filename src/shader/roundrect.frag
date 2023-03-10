#version 450

layout (set = 1, binding = 0) uniform ModelInfo {
    mat4 Model;
    vec4 FillColor;
    vec4 StrokeColor;
    vec2 Size;
    vec2 Radius;
    float StrokeWidth;
} modelInfo;

layout(location = 0) in vec2 inRectPos;

layout(location = 0) out vec4 outColor;

#define epsilon float(1e-37)

float sdfRoundedRectangle(vec2 pos, vec2 size, vec2 _radius){
    float radius = _radius.x;
    return length(max(abs(pos) - size + radius, 0.0)) - radius;
}


void main() {
    vec2 size = modelInfo.Size;
    vec2 radius = modelInfo.Radius;
    float strokeThickness = modelInfo.StrokeWidth;

    vec2 halfSize = (size / 2);
    vec2 rectPos = inRectPos - halfSize;

    float fillDistance = sdfRoundedRectangle(rectPos, halfSize - strokeThickness, radius);
    float strokeDistance = sdfRoundedRectangle(rectPos, halfSize, radius);

    float fillAlpha = 1 - step(epsilon, fillDistance);
    float strokeAlpha = 1 - step(epsilon, strokeDistance);
    strokeAlpha -= fillAlpha;


    outColor = modelInfo.FillColor * fillAlpha + modelInfo.StrokeColor * strokeAlpha;
}