#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
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

layout(set = 1, binding = 0) uniform DebugShapeInfo{
    mat4 modelMatrix;
    mat4 scaleMatrix;
    vec4 fill_color;
    vec4 outline_color;
    float edge_outline;
} gameObject;


void main() {
    vec4 positionWorld = gameObject.modelMatrix * vec4(position, 1.0);
    positionWorld.z = 5;
    gl_Position = ubo.projection * ubo.view * positionWorld;
    fragPosWorld = positionWorld.xyz;
    fragColor = gameObject.fill_color.xyz;
}
