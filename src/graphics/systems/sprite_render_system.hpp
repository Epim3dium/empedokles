#ifndef EMP_SPRITE_RENDER_SYSTEM_HPP
#define EMP_SPRITE_RENDER_SYSTEM_HPP
#include "graphics/camera.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/sprite_system.hpp"
#include "graphics/vulkan/descriptors.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/vulkan/pipeline.hpp"

// std
#include <memory>
#include <vector>
namespace emp {
class SpriteRenderSystem {
public:
    SpriteRenderSystem(
            Device& device,
            VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout,
            const char* frag_filename,
            const char* vert_filename
    );
    ~SpriteRenderSystem();
    SpriteRenderSystem(const SpriteRenderSystem&) = delete;
    SpriteRenderSystem& operator=(const SpriteRenderSystem&) = delete;

    void render(
            FrameInfo& frameInfo,
            const std::set<Entity>& entity_list,
            std::function<VkDescriptorBufferInfo(Entity, int)> getBufInfo,
            std::function<VkDescriptorImageInfo(Entity)> getDescImageInfo
    );

private:
    void createPipelineLayout(
            VkDescriptorSetLayout globalSetLayout,
            size_t push_constant_struct_size = 4
    );
    void createPipeline(
            VkRenderPass renderPass,
            const char* frag_filename,
            const char* vert_filename
    );

    Device& device;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout{};

    std::unique_ptr<DescriptorSetLayout> render_system_layout;
};
} // namespace emp

#endif
