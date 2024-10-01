#ifndef EMP_COLLIDER_HPP
#define EMP_COLLIDER_HPP
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "math/shapes/AABB.hpp"
#include "scene/transform.hpp"
#include <vector>
namespace emp {
class ColliderSystem;


struct Collider {
    typedef std::vector<vec2f> ConvexVertexCloud;
private:
    void m_updateNewTransform(const Transform& trans);
    AABB m_calcAABB() const;
    AABB m_aabb;
public:
    //potentially concave
    std::vector<vec2f> model_outline;
    std::vector<ConvexVertexCloud> model_shape;

    std::vector<vec2f> transformed_outline;
    std::vector<ConvexVertexCloud> transformed_shape;

    float area;
    float inertia_dev_mass;


    AABB aabb() const {return m_aabb; }
    Collider() {}
    Collider(std::vector<vec2f> shape, bool correctCOM = false);
    friend ColliderSystem;
};
//system for updating transfomred collider shapes
class ColliderSystem : public System<Transform, Collider> {
public:
    void update();
    void updateInstant(const Entity e);
};
};
#endif
