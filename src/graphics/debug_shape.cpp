#include "graphics/model.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "debug_shape.hpp"
#include "graphics/vertex.hpp"
#include "math/geometry_func.hpp"
#include "debug/log.hpp"
#include <numeric>


namespace emp {
    DebugShape::DebugShape(Device &device, std::vector<vec2f> verticies, bool isClosed) : m_outline(verticies), m_id("__DebugShape_" + std::to_string(getNextID())) {
        if(isClosed) {
            verticies.push_back(verticies.front());
        }
        ModelAsset::Builder builder;
        for(auto v : verticies) {
            Vertex vert;
            vert.position = glm::vec3(v, 0.f);
            builder.vertices.push_back(vert);
        }
        Model::create(device, builder, m_id.c_str());
    }
    DebugShape::~DebugShape() = default;

    uint32_t DebugShape::s_current_id = 0; 
};

