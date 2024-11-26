#include "model_system.hpp"
#include "core/coordinator.hpp"

#include <numeric>

namespace emp {

TexturedModelsSystem::TexturedModelsSystem(Device& device) {
    // including nonCoherentAtomSize allows us to flush a specific index at once
    int alignment = std::lcm(
            device.properties.limits.nonCoherentAtomSize,
            device.properties.limits.minUniformBufferOffsetAlignment
    );
    for (auto& uboBuffer : uboBuffers) {
        uboBuffer = std::make_unique<Buffer>(
                device,
                sizeof(TexturedModelInfo),
                MAX_ENTITIES,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                alignment
        );
        uboBuffer->map();
    }

    Texture::create("default", device, "../assets/textures/invalid.png");
}

void TexturedModelsSystem::updateBuffer(int frameIndex) {
    // copy model matrix and normal matrix for each gameObj into
    // buffer for this frame
    for (auto e : entities) {
        // auto &obj = kv.second;
        const auto& transform = getComponent<Transform>(e);
        TexturedModelInfo data{};
        data.modelMatrix = transform.global();
        data.hasTexture[0][0] = ECS().hasComponent<Texture>(e);
        uboBuffers[frameIndex]->writeToIndex(&data, e);
    }
    uboBuffers[frameIndex]->flush();
}
} // namespace emp
