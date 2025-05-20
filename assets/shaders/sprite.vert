#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec2 fragUv;

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

    vec4 color;
    vec4 color_override;

    vec2 rect_min;
    vec2 rect_max;
    vec2 flip; // only 0.f or 1.f
    float order;
    float _pad;
} sprite;


void main() {
    vec4 positionWorld = sprite.model_matrix * sprite.offset_matrix * sprite.pivot_matrix * sprite.size_matrix * vec4(position, 1.0);
    positionWorld = vec4(positionWorld.x, positionWorld.y, sprite.order, 1);
    gl_Position = ubo.projection * ubo.view * positionWorld;
    fragPosWorld = positionWorld.xyz;
    fragColor = vec4(color, 1);
    vec2 uv_range = sprite.rect_max - sprite.rect_min;
    vec2 uv_scaled = vec2(uv.x * uv_range.x, uv.y * uv_range.y);
    if(sprite.flip.x == 1) {
        uv_scaled  = vec2(uv_range.x - uv_scaled.x, uv_scaled.y);
    }
    if(sprite.flip.y == 1) {
        uv_scaled  = vec2(uv_scaled.x, uv_range.y - uv_scaled.y);
    }
    fragUv = uv_scaled + sprite.rect_min;
}
