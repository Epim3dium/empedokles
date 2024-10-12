#include "sprite_system.hpp"
namespace emp {
SpriteSystem::SpriteSystem(Device& device) {
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

void SpriteSystem::updateBuffer(int frameIndex) {
    // copy model matrix and normal matrix for each gameObj into
    // buffer for this frame
    for (auto e : entities) {
        // auto &obj = kv.second;
        const auto& transform = getComponent<Transform>(e);
        const auto& sprite = getComponent<SpriteRenderer>(e);
        SpriteInfo data{};

        data.model_matrix = transform.global();
        data.offset_matrix = Transform(
                                     sprite.sprite().position_offset,
                                     sprite.sprite().rotation_offset,
                                     sprite.sprite().scale_offset
        )
                                     .local();
        data.size_matrix = glm::scale(
                glm::mat4{1.f},
                {sprite.sprite().size().x, sprite.sprite().size().y, 1.f}
        );

        auto pivot = sprite.sprite().pivot;
        data.pivot_matrix = glm::translate(
                glm::mat4{1.f}, glm::vec3(pivot.x, pivot.y, 0.f)
        );

        data.rect_min = sprite.sprite().rect().min;
        data.rect_max = sprite.sprite().rect().max;
        data.flip = {sprite.flipX, sprite.flipY}; // only 0.f or 1.f
        data.color = sprite.color;

        uboBuffers[frameIndex]->writeToIndex(&data, e);
    }
    uboBuffers[frameIndex]->flush();
}
}; // namespace emp
