#ifndef EMP_DEBUG_SHAPE_RENDER_SYSTEM_HPP
#define EMP_DEBUG_SHAPE_RENDER_SYSTEM_HPP

#include "graphics/camera.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/vulkan/descriptors.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/vulkan/pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace emp {
class DebugShapeRenderSystem {
public:
    DebugShapeRenderSystem(
            Device& device,
            VkRenderPass renderPass,
            VkDescriptorSetLayout globalSetLayout,
            const char* frag_filename,
            const char* vert_filename
    );
    ~DebugShapeRenderSystem();
    DebugShapeRenderSystem(const DebugShapeRenderSystem&) = delete;
    DebugShapeRenderSystem& operator=(const DebugShapeRenderSystem&) = delete;

    void render(FrameInfo& frameInfo, DebugShapeSystem& model_sys);

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
    std::unique_ptr<Pipeline> outline_pipeline;
    VkPipelineLayout outline_pipeline_layout{};

    std::unique_ptr<Pipeline> fill_pipeline;
    VkPipelineLayout fill_pipeline_layout{};

    std::unique_ptr<DescriptorSetLayout> render_system_layout;
};
} // namespace emp
#endif
