#include <cstddef>
#include <functional>
#include <ranges>
#include "graphics/model.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <numeric>
#include "debug/log.hpp"
#include "debug_shape.hpp"
#include "graphics/vertex.hpp"
#include "math/geometry_func.hpp"

namespace emp {
void DebugShapeMesh::bindOutline(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {m_outline_vertex_buffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}
void DebugShapeMesh::drawOutline(VkCommandBuffer commandBuffer) const {
    vkCmdDraw(commandBuffer, m_outline_vertex_count, 1, 0, 0);
}
void DebugShapeMesh::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = {m_vertex_buffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (m_has_index_buffer) {
        vkCmdBindIndexBuffer(
                commandBuffer, m_index_buffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32
        );
    }
}
void DebugShapeMesh::draw(VkCommandBuffer commandBuffer) const {
    if (m_has_index_buffer) {
        vkCmdDrawIndexed(commandBuffer, m_index_count, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, m_vertex_count, 1, 0, 0);
    }
}
DebugShapeMesh::DebugShapeMesh(
    Device& device, const std::vector<vec2f>& vertices)
    : m_device(device) 
{
    std::vector<Vertex> graphic_vertices;
    for (auto v : vertices) {
        Vertex vert;
        vert.position = glm::vec3(v, 0.f);
        graphic_vertices.push_back(vert);
    }
    m_outline_vertex_buffer = createVertexBuffer(graphic_vertices, device);
    m_outline_vertex_count = vertices.size();

    if(vertices.front() == vertices.back()) {
        setupMesh(vertices, device);
    }
}
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}
std::size_t hash(vec2f v) {
    auto h = std::hash<float>()(v.x);
    hash_combine(h, v.y);
    return h;
}

void DebugShapeMesh::setupMesh(std::vector<vec2f> verticies, Device& device) {
    auto triangles = triangulate(verticies);

    std::vector<vec2f> flattened_triangles(triangles.size() * 3U);
    std::vector<uint32_t> indices;

    for(int i = 0; i < triangles.size(); i++) {
        for(int ii = 0; ii < 3; ii++) {
            flattened_triangles[i * 3U + ii] = triangles[i].arr[ii];
        }
    }
    std::vector<Vertex> graphic_vertices;
    std::unordered_map<std::size_t, uint32_t> hash_to_index;
    size_t cur_index = 0;
    for(int i = 0; i < flattened_triangles.size(); i++) {
        auto vec = flattened_triangles[i];
        auto hash_vec = hash(vec);
        if(!hash_to_index.contains(hash_vec)) {
            hash_to_index[hash_vec] = cur_index++;

            Vertex vert;
            vert.position = vec3f(vec, 0.f);
            graphic_vertices.push_back(vert);
        }
        indices.push_back(hash_to_index.at(hash_vec));
    }
    m_vertex_buffer = createVertexBuffer(graphic_vertices, device);
    m_vertex_count = graphic_vertices.size();

    m_index_buffer = createIndexBuffer(indices, device);
    m_index_count = indices.size();
    m_has_index_buffer = true;
}
std::unique_ptr<Buffer> DebugShapeMesh::createVertexBuffer(
    const std::vector<Vertex>& vertices, Device& device) {
    uint32_t vertex_count = static_cast<uint32_t>(vertices.size());
    assert(vertex_count >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertex_count;
    uint32_t vertexSize = sizeof(vertices[0]);

    Buffer stagingBuffer{
            device,
            vertexSize,
            vertex_count,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)vertices.data());

    auto result = std::make_unique<Buffer>(
            device,
            vertexSize,
            vertex_count,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    device.copyBuffer(
            stagingBuffer.getBuffer(), result->getBuffer(), bufferSize
    );
    return std::move(result);
}
std::unique_ptr<Buffer> DebugShapeMesh::createIndexBuffer(
    const std::vector<uint32_t>& indices, Device& device) {
    uint32_t indexCount = static_cast<uint32_t>(indices.size());
    bool hasIndexBuffer = indexCount > 0;

    if (!hasIndexBuffer) {
        return nullptr;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer{
            device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)indices.data());

    auto result= std::make_unique<Buffer>(
            device,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    device.copyBuffer(
            stagingBuffer.getBuffer(), result->getBuffer(), bufferSize
    );
    return result;
}
DebugShape::DebugShape(
        Device& device,
        std::vector<vec2f> verticies,
        glm::vec4 fill,
        glm::vec4 outline,
        bool isClosed
)
    : m_outline(verticies), fill_color(fill), outline_color(outline) {
    if (isClosed) {
        verticies.push_back(verticies.front());
    }
    m_mesh = std::make_shared<DebugShapeMesh>(device, verticies);
}
DebugShape::~DebugShape() = default;
}; // namespace emp
