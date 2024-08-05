#pragma once
#include "math/math_defs.hpp"
#include "vulkan/vulkan_core.h"
#include <vector>
namespace emp {
    struct Vertex {
        glm::vec3 position{};
        glm::vec3 color{};
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex &other) const {
            return position == other.position && color == other.color && normal == other.normal &&
                   uv == other.uv;
        }
    };
};
