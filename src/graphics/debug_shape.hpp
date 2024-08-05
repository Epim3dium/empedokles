#pragma once
#include "vulkan/buffer.hpp"
#include "vulkan/device.hpp"
#include <unordered_map>
#include "math/math_defs.hpp"

namespace emp {
    class DebugShape {
    public:
        glm::vec4 fill_color;
        glm::vec4 outline_color;
        DebugShape() {}
        DebugShape(Device &device, std::vector<vec2f> verticies);
        ~DebugShape();

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer) const;
    private:
        void createVertexBuffers(const std::vector<vec2f> &vertices, Device& device);

        std::shared_ptr<Buffer> vertexBuffer{};
        uint32_t vertexCount{};
    };
};
