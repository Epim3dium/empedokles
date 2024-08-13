#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "debug_shape.hpp"
#include "graphics/vertex.hpp"
#include "math/geometry_func.hpp"
#include "debug/log.hpp"
#include <numeric>


namespace emp {
    DebugShape::DebugShape(Device &device, std::vector<vec2f> verticies, bool isClosed) : m_outline(verticies) {
        if(isClosed) {
            verticies.push_back(verticies.front());
        }
        createVertexBuffers(verticies, device);
        //createIndexBuffers(indicies, device);
    }
    DebugShape::~DebugShape() = default;
    void DebugShape::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        // vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
    void DebugShape::draw(VkCommandBuffer commandBuffer) const {
        // vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
    
    void DebugShape::createIndexBuffers(const std::vector<uint32_t> &indices, Device& device) {
        indexCount = static_cast<uint32_t>(indices.size());

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) indices.data());

        indexBuffer = std::make_shared<Buffer>(
                device,
                indexSize,
                indexCount,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }
    void DebugShape::createVertexBuffers(const std::vector<vec2f> &vertices_positions, Device& device) {
        vertexCount = static_cast<uint32_t>(vertices_positions.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(Vertex) * vertexCount;
        uint32_t vertexSize = sizeof(Vertex);

        Buffer stagingBuffer{
            device,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        std::vector<Vertex> verticies(vertices_positions.size());
        for(int i = 0; i < vertices_positions.size(); i++) {
            verticies[i].position = {vertices_positions[i].x, vertices_positions[i].y, 0.f};
        }

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) verticies.data());

        vertexBuffer = std::make_shared<Buffer>(
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
};

