#ifndef EMP_DEBUG_SHAPE_HPP
#define EMP_DEBUG_SHAPE_HPP
#include "graphics/model.hpp"
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
        DebugShape(Device &device, std::vector<vec2f> verticies, glm::vec4 fill = glm::vec4(1), glm::vec4 outline = glm::vec4(1), bool isClosed = true);
        ~DebugShape();

        const std::vector<vec2f>& outline() const { return m_outline; }
        ModelAsset& model() {
            return *m_model;
        }
    private:
        std::vector<vec2f> m_outline;
        std::shared_ptr<ModelAsset> m_model;
    };
};
#endif
