#pragma once

#include "graphics/camera.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/vulkan/pipeline.hpp"
#include "graphics/vulkan/descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace emp {
    class DebugShapeRenderSystem {
    public:
        DebugShapeRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, const char* frag_filename, const char* vert_filename);
        ~DebugShapeRenderSystem();
        DebugShapeRenderSystem(const DebugShapeRenderSystem &) = delete;
        DebugShapeRenderSystem &operator=(const DebugShapeRenderSystem &) = delete;

        void render(FrameInfo &frameInfo, DebugShapeSystem& model_sys);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, size_t push_constant_struct_size = 4);
        void createPipeline(VkRenderPass renderPass, const char* frag_filename, const char* vert_filename);

        Device &device;
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipeline_layout{};

        std::unique_ptr<DescriptorSetLayout> render_system_layout;
    };
}  // namespace emp