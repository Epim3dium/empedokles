#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec2 fragUv;

layout (location = 0) out vec4 outColor;

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

layout(set = 1, binding = 0) uniform SpriteInfo {
    mat4 model_matrix;
    mat4 offset_matrix;
    mat4 pivot_matrix;
    mat4 size_matrix;

    vec2 rect_min;
    vec2 rect_max;
    vec2 flip; // only 0.f or 1.f
       
    vec4 color;
} sprite;
layout (set = 1, binding = 1) uniform sampler2D diffuseTex;

void main() {
    vec2 uv = fragUv;
    if(sprite.flip.x == 1) {
        uv.x = 1 - uv.x;
    }
    if(sprite.flip.y == 1) {
        uv.y = 1 - uv.y;
    }
    vec3 color = texture(diffuseTex, uv).xyz;
    outColor = vec4(color, 1);
}
