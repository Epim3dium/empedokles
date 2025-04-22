#version 450
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};
layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor; // w is intensity
    PointLight pointLights[10];
    int numLights;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
    gl_PointSize = 1.0;
}
