#ifndef EMP_ANIMATED_SPRITE_SYSTEM
#define EMP_ANIMATED_SPRITE_SYSTEM
#include "graphics/animated_sprite.hpp"
#include "graphics/vulkan/swap_chain.hpp"
#include "scene/transform.hpp"
namespace emp {
    class AnimatedSpriteSystem : public System<AnimatedSprite, Transform> {
    public:
        AnimatedSpriteSystem(Device& device);

        [[nodiscard]] VkDescriptorBufferInfo getBufferInfoForGameObject(
                int frameIndex, Entity entity
        ) const {
            return uboBuffers[frameIndex]->descriptorInfoForIndex(entity);
        }

        void updateTransitions();
        void updateBuffer(int frameIndex);
        std::vector<std::unique_ptr<Buffer>> uboBuffers{
                SwapChain::MAX_FRAMES_IN_FLIGHT
        };
    };
};
#endif //EMP_ANIMATED_SPRITE_SYSTEM
