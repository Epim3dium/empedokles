#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragPosWorld;

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

layout(set = 1, binding = 0) uniform ModelInfo {
    mat4 modelMatrix;
    mat4 normalMatrix;
    vec4 color;
} model;


void main() {
    vec4 positionWorld = model.modelMatrix * vec4(position, 1.0);
    positionWorld.z = -0.1;
    gl_Position = ubo.projection * ubo.view * positionWorld;
    fragPosWorld = positionWorld.xyz;
    fragColor = model.color;
}
