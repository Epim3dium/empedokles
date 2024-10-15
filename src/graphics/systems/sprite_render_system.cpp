#include "sprite_render_system.hpp"
#include "core/coordinator.hpp"
#include "graphics/debug_shape.hpp"
#include "simple_render_system.hpp"

#include "debug_shape_render_system.hpp"
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

SpriteRenderSystem::SpriteRenderSystem(
        Device& device,
        VkRenderPass renderPass,
        VkDescriptorSetLayout globalSetLayout,
        const char* vert_filename,
        const char* frag_filename
)
    : device{device} {
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass, vert_filename, frag_filename);
}

SpriteRenderSystem::~SpriteRenderSystem() {
    vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
}

void SpriteRenderSystem::createPipelineLayout(
        VkDescriptorSetLayout globalSetLayout, size_t push_const_size
) {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags =
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = push_const_size;

    render_system_layout =
            DescriptorSetLayout::Builder(device)
                    .addBinding(
                            0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_VERTEX_BIT |
                                    VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .addBinding(
                            1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .build();

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalSetLayout, render_system_layout->getDescriptorSetLayout()
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount =
            static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = push_const_size ? 1 : 0;
    pipelineLayoutInfo.pPushConstantRanges =
            push_const_size ? &pushConstantRange : nullptr;
    if (vkCreatePipelineLayout(
                device.device(), &pipelineLayoutInfo, nullptr, &pipeline_layout
        ) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void SpriteRenderSystem::createPipeline(
        VkRenderPass renderPass,
        const char* vert_filename,
        const char* frag_filename
) {
    assert(pipeline_layout != nullptr &&
           "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};
    Pipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;

    pipelineConfig.pipelineLayout = pipeline_layout;
    pipeline = std::make_unique<Pipeline>(
            device, vert_filename, frag_filename, pipelineConfig
    );
}

void SpriteRenderSystem::render(
    FrameInfo& frameInfo,
    const std::set<Entity>& entity_list,
    std::function<VkDescriptorBufferInfo(Entity, int)> getBufInfo,
    std::function<VkDescriptorImageInfo(Entity)> getDescImageInfo
) {
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

    for (auto entity : entity_list) {

        // writing descriptor set each frame can slow performance
        // would be more efficient to implement some sort of caching
        auto buffer_info =
                getBufInfo(entity, frameInfo.frameIndex);
        VkDescriptorImageInfo image_info;
        VkDescriptorSet entity_desc_set;

        DescriptorWriter desc_writer(
                *render_system_layout, frameInfo.frameDescriptorPool
        );

        desc_writer.writeBuffer(0, &buffer_info);

        image_info = getDescImageInfo(entity);
        desc_writer.writeImage(1, &image_info);

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

        Sprite::bind(frameInfo.commandBuffer);
        Sprite::draw(frameInfo.commandBuffer);
    }
}

}; // namespace emp

