#include "debug_shape_system.hpp"
#include "core/coordinator.hpp"

#include <numeric>

namespace emp {

    DebugShapeSystem::DebugShapeSystem(Device &device) {
        // including nonCoherentAtomSize allows us to flush a specific index at once
        int alignment = std::lcm(
                device.properties.limits.nonCoherentAtomSize,
                device.properties.limits.minUniformBufferOffsetAlignment);
        for (auto &uboBuffer: uboBuffers) {
            uboBuffer = std::make_unique<Buffer>(
                    device,
                    sizeof(DebugShapeInfo),
                    MAX_ENTITIES,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    alignment);
            uboBuffer->map();
        }
    }

    void DebugShapeSystem::updateBuffer(int frameIndex) {
        // copy model matrix and normal matrix for each gameObj into
        // buffer for this frame
        for (auto e : entities) {
            // auto &obj = kv.second;
            const auto& transform = getComponent<Transform>(e);
            const auto& debug_shape = getComponent<DebugShape>(e);
            DebugShapeInfo data{};
            data.modelMatrix = transform.global();
            data.fill_color = debug_shape.fill_color;
            data.outline_color = debug_shape.outline_color;
            uboBuffers[frameIndex]->writeToIndex(&data, e);
        }
        uboBuffers[frameIndex]->flush();
    }
} 