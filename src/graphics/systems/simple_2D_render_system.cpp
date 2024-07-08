#include "simple_2D_render_system.hpp"
#include "core/coordinator.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace emp {

    Simple2DColorRenderSystem::Simple2DColorRenderSystem(
            Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
            : device{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    Simple2DColorRenderSystem::~Simple2DColorRenderSystem() {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void Simple2DColorRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

        renderSystemLayout =
            DescriptorSetLayout::Builder(device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT |
                                VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout, renderSystemLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void Simple2DColorRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(
                device,
                "assets/shaders/2D_simplest_color_shader.vert.spv",
                "assets/shaders/2D_simplest_color_shader.frag.spv",
                pipelineConfig);
    }

    void Simple2DColorRenderSystem::render(FrameInfo &frameInfo, TexturedModelsSystem& model_sys) {
        pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                1,
                &frameInfo.globalDescriptorSet,
                0,
                nullptr);

        for (auto e: frameInfo.gameObjects) {
            if (!coordinator.hasComponent<Model>(e)) continue;
            auto& model = coordinator.getComponent<Model>(e);

            // writing descriptor set each frame can slow performance
            // would be more efficient to implement some sort of caching
            auto buffer_info = model_sys.getBufferInfoForGameObject(frameInfo.frameIndex, e);
            VkDescriptorSet entity_desc_set;

            DescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
                .writeBuffer(0, &buffer_info)
                .build(entity_desc_set);

            vkCmdBindDescriptorSets(
                    frameInfo.commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout,
                    1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
                    1,  // set count
                    &entity_desc_set,
                    0,
                    nullptr);

            model.model().bind(frameInfo.commandBuffer);
            model.model().draw(frameInfo.commandBuffer);
        }
    }

}  // namespace emp
