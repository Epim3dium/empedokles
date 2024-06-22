#ifndef EMP_COLLIDER_HPP
#define EMP_COLLIDER_HPP
#include "core/component_system.hpp"
#include "math/math_defs.hpp"
#include "core/transform.hpp"
#include <vector>
namespace emp {
class Collider;

typedef ComponentSystem<Collider> ColliderSystem;
typedef ComponentInstance<Collider, ColliderSystem> ColliderInstance;

class Collider {
    typedef std::vector<vec2f> ConvexVertexCloud;
private:
    //potentially concave
    std::vector<vec2f> m_model_outline;
    std::vector<ConvexVertexCloud> m_model_shape;

    std::vector<vec2f> m_transformed_outline;
    std::vector<ConvexVertexCloud> m_transformed_shape;

    emp::Transform* m_transform;
    float m_area;
    float m_inertia_dev_mass;

    void m_calcModelShape();
public:
    inline const std::vector<vec2f>& outlineModel() const {
        return m_model_outline;
    }
    inline const std::vector<ConvexVertexCloud>& constituentConvexModel() const {
        return m_model_shape;
    }
    inline const std::vector<vec2f>& outline() const {
        return m_transformed_outline;
    }
    inline const std::vector<ConvexVertexCloud>& constituentConvex() const {
        return m_transformed_shape;
    }

    inline float area() const { return m_area; }
    inline float inertiaDevMass() const { return m_inertia_dev_mass; }

    void update();
    
    Collider(const Collider&) = delete;
    Collider(Collider&&) = delete;
    Collider& operator=(const Collider&) = delete;
private:
    Collider(std::vector<vec2f> shape, emp::Transform* trans, bool correctCOM = false);
    friend ColliderSystem;
};
};
#endif
