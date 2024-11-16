#ifndef EMP_DEBUG_SHAPE_HPP
#define EMP_DEBUG_SHAPE_HPP
#include <unordered_map>
#include "graphics/model.hpp"
#include "math/math_defs.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/device.hpp"

namespace emp {
class DebugShapeMesh {
public:
    void bindOutline(VkCommandBuffer commandBuffer);
    void drawOutline(VkCommandBuffer commandBuffer) const;
    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer) const;

    DebugShapeMesh(Device& device, const std::vector<vec2f>& vertices);
    DebugShapeMesh(const DebugShapeMesh&) = delete; 
    DebugShapeMesh& operator=(const DebugShapeMesh&) = delete; 
private:
    Device& m_device;
    std::unique_ptr<Buffer> createVertexBuffer(
            const std::vector<Vertex>& vertices, Device& device
    );
    void setupMesh(std::vector<vec2f> vertices, Device& device);
    std::unique_ptr<Buffer> createIndexBuffer(
            const std::vector<uint32_t>& indices, Device& device
    );

    std::unique_ptr<Buffer> m_outline_vertex_buffer;
    uint32_t m_outline_vertex_count{};

    std::unique_ptr<Buffer> m_vertex_buffer;
    uint32_t m_vertex_count{};

    bool m_has_index_buffer = false;
    std::unique_ptr<Buffer> m_index_buffer;
    uint32_t m_index_count{};
};
class DebugShape {
public:
    glm::vec4 fill_color = {0, 0, 0, 0};
    glm::vec4 outline_color = {0, 0, 0, 0};
    float outline_width = 1.f;
    DebugShape() {
    }
    DebugShape(
            Device& device,
            std::vector<vec2f> verticies,
            glm::vec4 fill = glm::vec4(1),
            glm::vec4 outline = glm::vec4(1),
            bool isClosed = true
    );
    ~DebugShape();

    const std::vector<vec2f>& outline() const {
        return m_outline;
    }
    DebugShapeMesh& mesh() {
        return *m_mesh;
    }

private:
    std::vector<vec2f> m_outline;
    std::shared_ptr<DebugShapeMesh> m_mesh;
};
}; // namespace emp
#endif
