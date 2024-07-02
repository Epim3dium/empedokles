#pragma once

#include "graphics/camera.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/game_object.hpp"
#include "graphics/vulkan/pipeline.hpp"
#include "graphics/vulkan/descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace emp {
    class Simple2DColorRenderSystem {
    public:
        Simple2DColorRenderSystem(
                Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

        ~Simple2DColorRenderSystem();

        Simple2DColorRenderSystem(const Simple2DColorRenderSystem &) = delete;

        Simple2DColorRenderSystem &operator=(const Simple2DColorRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);

        void createPipeline(VkRenderPass renderPass);

        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipelineLayout{};

        std::unique_ptr<DescriptorSetLayout> renderSystemLayout;
    };
}  // namespace emp
