#include "animated_sprite_system.hpp"
#include "graphics/sprite_system.hpp"

namespace emp {
    AnimatedSpriteSystem::AnimatedSpriteSystem(Device& device) {
        // including nonCoherentAtomSize allows us to flush a specific index at once
        int alignment = std::lcm(
                device.properties.limits.nonCoherentAtomSize,
                device.properties.limits.minUniformBufferOffsetAlignment
        );
        for (auto& uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(SpriteInfo),
                    MAX_ENTITIES,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    alignment
            );
            uboBuffer->map();
        }
    }

    void AnimatedSpriteSystem::updateTransitions() {
        for (auto entity : entities) {
            getComponent<AnimatedSprite>(entity).updateState(entity);
        }
    }
    void AnimatedSpriteSystem::updateBuffer(int frameIndex) {
        // copy model matrix and normal matrix for each gameObj into
        // buffer for this frame
        for (auto entity : entities) {

            // auto &obj = kv.second;
            const auto& transform = getComponent<Transform>(entity);
            const auto& animated = getComponent<AnimatedSprite>(entity);
            SpriteInfo data{};

            data.model_matrix = transform.global();
            data.offset_matrix = glm::translate(
                    glm::mat4x4(1.f), glm::vec3(animated.sprite().position_offset + animated.position_offset, 0)
            );
            data.size_matrix = glm::scale(
                    glm::mat4{1.f}, {animated.sprite().size().x, animated.sprite().size().y, 1.f}
            );

            auto pivot = (animated.sprite().centered ? vec2f(0, 0) : animated.sprite().size() * 0.5f);
            data.pivot_matrix = glm::translate(
                    glm::mat4{1.f}, glm::vec3(pivot.x, pivot.y, 0.f)
            );

            auto rect = animated.sprite().shader_rect();
            data.rect_min = rect.min;
            data.rect_max = rect.max;

            data.flip = {animated.sprite().flipX ^ animated.flipX, animated.sprite().flipY ^ animated.flipY}; // only 0.f or 1.f
            data.color = animated.sprite().color;

            uboBuffers[frameIndex]->writeToIndex(&data, entity);
        }
        uboBuffers[frameIndex]->flush();
    }
};
