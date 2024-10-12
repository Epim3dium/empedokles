#ifndef EMP_SPRITE_SYSTEM_HPP
#define EMP_SPRITE_SYSTEM_HPP
#include "graphics/sprite.hpp"
#include "scene/transform.hpp"
#include "vulkan/swap_chain.hpp"
namespace emp {
struct SpriteInfo {
    glm::mat4 model_matrix{1.f};
    glm::mat4 offset_matrix{1.f};
    glm::mat4 pivot_matrix{1.f};
    glm::mat4 size_matrix{1.f};

    glm::vec2 rect_min;
    glm::vec2 rect_max;
    glm::vec2 flip; // only 0.f or 1.f

    glm::vec4 color;
};
struct SpriteSystem : public System<Sprite, Transform> {
    SpriteSystem(Device& device);

    [[nodiscard]] VkDescriptorBufferInfo getBufferInfoForGameObject(
            int frameIndex, Entity entity
    ) const {
        return uboBuffers[frameIndex]->descriptorInfoForIndex(entity);
    }

    void updateBuffer(int frameIndex);
    std::vector<std::unique_ptr<Buffer>> uboBuffers{
            SwapChain::MAX_FRAMES_IN_FLIGHT
    };
};
}; // namespace emp
#endif

