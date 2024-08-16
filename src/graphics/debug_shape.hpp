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
        DebugShape(Device &device, std::vector<vec2f> verticies, bool isClosed = true);
        ~DebugShape();

        const std::vector<vec2f>& outline() const { return m_outline; }
        ModelAsset& model() {
            return Model(m_id.c_str()).model();
        }
    private:
        std::string m_id;
        static uint32_t getNextID() {
            return s_current_id++;
        }
        static uint32_t s_current_id; 
        std::vector<vec2f> m_outline;
    };
};
#endif
