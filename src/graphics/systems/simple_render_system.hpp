#ifndef EMP_SIMPLE_RENDER_SYSTEM_HPP
#define EMP_SIMPLE_RENDER_SYSTEM_HPP

#include "graphics/camera.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/vulkan/descriptors.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/vulkan/pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace emp {
class SimpleRenderSystem {
public:
    SimpleRenderSystem(
            Device& device,
            VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout,
            const char* frag_filename,
            const char* vert_filename,
            PipelineConfigInfo* config = nullptr
    );
    ~SimpleRenderSystem();
    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

    template <class IterableObjects> 
    void render(FrameInfo& frameInfo,
           IterableObjects& objects,
           std::function<bool(DescriptorWriter&, int frame_idx, const typename IterableObjects::value_type&)> writeDesc,
           std::function<void(const VkCommandBuffer&, const typename IterableObjects::value_type&)> bindAndDraw);

private:
    void createPipelineLayout(
            VkDescriptorSetLayout globalSetLayout,
            size_t push_constant_struct_size = 4
    );
    void createPipeline(
            VkRenderPass renderPass,
            const char* frag_filename,
            const char* vert_filename,
            PipelineConfigInfo* config = nullptr
    );

    Device& device;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout{};

    std::unique_ptr<DescriptorSetLayout> render_system_layout;
};
template <class IterableObjects> 
void SimpleRenderSystem::render(FrameInfo& frameInfo,
       IterableObjects& objects,
       std::function<bool(DescriptorWriter&, int frame_idx, const typename IterableObjects::value_type&)> writeDesc,
       std::function<void(const VkCommandBuffer&, const typename IterableObjects::value_type&)> bindAndDraw) 
{
    pipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
    );

    for (auto object : objects) {
        // writing descriptor set each frame can slow performance
        // would be more efficient to implement some sort of caching
        VkDescriptorImageInfo image_info;

        DescriptorWriter desc_writer(
                *render_system_layout, frameInfo.frameDescriptorPool
        );

         
        if(!writeDesc(desc_writer, frameInfo.frameIndex, object)) {
            continue;
        }
        VkDescriptorSet entity_desc_set;
        desc_writer.build(entity_desc_set);

        vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_layout,
                1, // starting set (0 is the globalDescriptorSet, 1 is the set
                   // specific to this system)
                1, // set count
                &entity_desc_set,
                0,
                nullptr
        );

        bindAndDraw(frameInfo.commandBuffer, object);
    }

}
} // namespace emp
#endif
