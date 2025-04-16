#ifndef EMP_RENDERER_CONTEXT_HPP
#define EMP_RENDERER_CONTEXT_HPP
#include <memory>
#include "graphics/renderer.hpp"
#include "vulkan/descriptors.hpp"
#include "graphics/render_systems/simple_render_system.hpp"
#include "compute/compute_manager.hpp"
#include "scene/compute_demo.hpp"
namespace emp {
struct RendererContext {
    std::unique_ptr<DescriptorPool> globalPool;
    std::vector<std::unique_ptr<DescriptorPool>> frame_pools;
    std::unique_ptr<SimpleRenderSystem> sprite_rend_sys;
    std::unique_ptr<SimpleRenderSystem> model_rend_sys;
    // std::unique_ptr<ComputeDemo> compute_demo;

    std::vector<std::unique_ptr<Buffer>> ubo_buffers;
    std::vector<std::unique_ptr<Buffer>> ubo_compute_buffers;
    std::unique_ptr<DescriptorSetLayout> global_set_layout;
    std::unique_ptr<DescriptorSetLayout> compute_set_layout;

    std::vector<VkDescriptorSet> global_compute_descriptor_sets;
    std::vector<VkDescriptorSet> global_descriptor_sets;
    
    void setup(Device& device, Renderer& renderer) {
        globalPool = DescriptorPool::Builder(device)
                             .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
                             .addPoolSize(
                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     SwapChain::MAX_FRAMES_IN_FLIGHT
                             )
                             .build();

        // build frame descriptor pools
        frame_pools.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
        auto framePoolBuilder =
                DescriptorPool::Builder(device)
                        .setMaxSets(1000)
                        .addPoolSize(
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000
                        )
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                        .setPoolFlags(
                                VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
                        );
        for (auto& framePool : frame_pools) {
            framePool = framePoolBuilder.build();
        }
        ubo_compute_buffers = m_setupGlobalUBOBuffers(device);
        ubo_buffers = m_setupGlobalUBOBuffers(device);
        global_set_layout = DescriptorSetLayout::Builder(device)
                                   .addBinding(0,
                                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                       VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .build();
        compute_set_layout = DescriptorSetLayout::Builder(device)
                                     .addBinding(0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_ALL_GRAPHICS)
                                     .build();

        global_descriptor_sets =
                setupGlobalUBODescriptorSets(*global_set_layout, ubo_buffers, *globalPool);
        global_compute_descriptor_sets =
            setupGlobalUBODescriptorSets(*compute_set_layout, ubo_compute_buffers, *globalPool);

        {
            PipelineConfigInfo debug_shape_pipeline_config;
            Pipeline::defaultPipelineConfigInfo(debug_shape_pipeline_config);
            Pipeline::enableAlphaBlending(debug_shape_pipeline_config);
            model_rend_sys = std::make_unique<SimpleRenderSystem>(device,
                renderer.getSwapChainRenderPass(),
                global_set_layout->getDescriptorSetLayout(),
                "../assets/shaders/debug_shape.vert.spv",
                "../assets/shaders/debug_shape.frag.spv",
                &debug_shape_pipeline_config);
        }
        sprite_rend_sys = std::make_unique<SimpleRenderSystem>(device,
            renderer.getSwapChainRenderPass(),
            global_set_layout->getDescriptorSetLayout(),
            "../assets/shaders/sprite.vert.spv",
            "../assets/shaders/sprite.frag.spv");
        // compute_demo = std::make_unique<ComputeDemo>(device);
    }
    std::vector<VkDescriptorSet> setupGlobalUBODescriptorSets(
            DescriptorSetLayout& globalSetLayout,
            const std::vector<std::unique_ptr<Buffer>>& uboBuffers,
            DescriptorPool& global_pool
    ) {
        std::vector<VkDescriptorSet> globalDescriptorSets(
                SwapChain::MAX_FRAMES_IN_FLIGHT
        );
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(globalDescriptorSets[i]);
        }
        return globalDescriptorSets;
    }
    std::vector<std::unique_ptr<Buffer>> m_setupGlobalComputeUBOBuffers(Device& device) {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(
                SwapChain::MAX_FRAMES_IN_FLIGHT
        );
        for (auto& uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(GlobalComputeUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffer->map();
        }
        return uboBuffers;
    }
    std::vector<std::unique_ptr<Buffer>> m_setupGlobalUBOBuffers(Device& device) {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(
                SwapChain::MAX_FRAMES_IN_FLIGHT
        );
        for (auto& uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(GlobalUbo),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffer->map();
        }
        return uboBuffers;
    }
};
}
#endif // !DEBUG
