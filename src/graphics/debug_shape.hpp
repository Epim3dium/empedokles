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

        const std::vector<vec2f>& outline() const { return m_outline; }
    private:
        std::vector<vec2f> m_outline;
        void createVertexBuffers(const std::vector<vec2f> &vertices, Device& device);
        void createIndexBuffers(const std::vector<uint32_t> &indices, Device& device);

        std::shared_ptr<Buffer> vertexBuffer{};
        std::shared_ptr<Buffer> indexBuffer{};
        uint32_t vertexCount{};
        uint32_t indexCount{};
    };
};
