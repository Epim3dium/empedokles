#pragma once

#include "model.hpp"
#include "scene/transform.hpp"
#include "texture.hpp"
#include "vulkan/swap_chain.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace emp {

    struct TexturedModelInfo {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
        glm::mat4 hasTexture;
    };

    class TexturedModelsSystem : public System<Transform, Model> {
    public:
        TexturedModelsSystem(Device &device);

        [[nodiscard]] VkDescriptorBufferInfo getBufferInfoForGameObject(
                int frameIndex, Entity entity) const {
            return uboBuffers[frameIndex]->descriptorInfoForIndex(entity);
        }

        void updateBuffer(int frameIndex);
        std::vector<std::unique_ptr<Buffer>> uboBuffers{SwapChain::MAX_FRAMES_IN_FLIGHT};
    };

}  
