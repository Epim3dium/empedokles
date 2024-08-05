#ifndef EMP_TRANSFORM_HPP
#define EMP_TRANSFORM_HPP
#include "core/system.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "math/math_defs.hpp"
#include <vector>

namespace emp {
class TransformSystem;
class Transform {
    TransformMatrix m_local_transform;
    TransformMatrix m_global_transform;

    void m_updateLocalTransform();
    void m_syncWithChange();
public:
    vec2f position = vec2f(0.f, 0.f);
    float rotation = 0.f;
    vec2f scale = vec2f(0.f, 0.f);

    void setPositionNow(vec2f p);
    void setRotationNow(float r);
    void setScaleNow(vec2f s);
    inline const TransformMatrix& local() const {
        return m_local_transform;
    }
    inline const TransformMatrix& global() const {
        return m_global_transform;
    }
    Transform() {}
    Transform(vec2f pos, float rot = 0.f, vec2f s = {1.f, 1.f}) : position(pos), rotation(rot), scale(s) { }
    friend TransformSystem;
};
class TransformSystem : public SystemOf<Transform> {
public:
    void update();
};
};
#endif
