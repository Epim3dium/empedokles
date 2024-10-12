#include "graphics/model.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <numeric>
#include "debug/log.hpp"
#include "debug_shape.hpp"
#include "graphics/vertex.hpp"
#include "math/geometry_func.hpp"

namespace emp {
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
    ModelAsset::Builder builder;
    for (auto v : verticies) {
        Vertex vert;
        vert.position = glm::vec3(v, 0.f);
        builder.vertices.push_back(vert);
    }
    m_model = std::make_shared<ModelAsset>(device, builder);
}
DebugShape::~DebugShape() = default;
}; // namespace emp
