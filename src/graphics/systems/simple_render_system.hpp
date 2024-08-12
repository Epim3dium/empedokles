#ifndef EMP_SIMPLE_RENDER_SYSTEM_HPP
#define EMP_SIMPLE_RENDER_SYSTEM_HPP

#include "graphics/camera.hpp"
#include "graphics/vulkan/device.hpp"
#include "graphics/frame_info.hpp"
#include "graphics/model_system.hpp"
#include "graphics/vulkan/pipeline.hpp"
#include "graphics/vulkan/descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace emp {
    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, const char* frag_filename, const char* vert_filename);
        ~SimpleRenderSystem();
        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void render(FrameInfo &frameInfo, TexturedModelsSystem& model_sys);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, size_t push_constant_struct_size = 4);
        void createPipeline(VkRenderPass renderPass, const char* frag_filename, const char* vert_filename);

        Device &device;
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout pipeline_layout{};

        std::unique_ptr<DescriptorSetLayout> render_system_layout;
    };
}  // namespace emp
#endif
