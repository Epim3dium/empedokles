#include "model_system.hpp"
#include "core/coordinator.hpp"

#include <numeric>

namespace emp {

    TexturedModelsSystem::TexturedModelsSystem(Device &device) {
        // including nonCoherentAtomSize allows us to flush a specific index at once
        int alignment = std::lcm(
                device.properties.limits.nonCoherentAtomSize,
                device.properties.limits.minUniformBufferOffsetAlignment);
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(TexturedModelInfo),
                    MAX_ENTITIES,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    alignment);
            uboBuffer->map();
        }

        textureDefault = TextureAsset::createTextureFromFile(device, "../assets/textures/star.jpg");
    }

    void TexturedModelsSystem::updateBuffer(int frameIndex) {
        // copy model matrix and normal matrix for each gameObj into
        // buffer for this frame
        for (auto e : entities) {
            // auto &obj = kv.second;
            const auto& transform = coordinator.getComponent<Transform2D>(e);
            TexturedModelInfo data{};
            data.modelMatrix = transform.global();
            data.hasTexture[0][0] = coordinator.hasComponent<Texture>(e);
            uboBuffers[frameIndex]->writeToIndex(&data, e);
        }
        uboBuffers[frameIndex]->flush();
    }
} 
