#ifndef EMP_DEBUGSHAPE_SYS
#define EMP_DEBUGSHAPE_SYS

#include "graphics/debug_shape.hpp"
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

    struct DebugShapeInfo {
        glm::mat4 modelMatrix{1.f};
        glm::vec4 fill_color;
        glm::vec4 outline_color;
    };

    class DebugShapeSystem : public System<Transform, DebugShape> {
    public:
        DebugShapeSystem(Device &device);

        [[nodiscard]] VkDescriptorBufferInfo getBufferInfoForGameObject(
                int frameIndex, Entity entity) const {
            return uboBuffers[frameIndex]->descriptorInfoForIndex(entity);
        }

        void updateBuffer(int frameIndex);
        std::vector<std::unique_ptr<Buffer>> uboBuffers{SwapChain::MAX_FRAMES_IN_FLIGHT};
    };

}  
#endif
