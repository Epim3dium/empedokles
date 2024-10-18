#ifndef EMP_COLLIDER_HPP
#define EMP_COLLIDER_HPP
#include <vector>
#include "core/layer.hpp"
#include "core/system.hpp"
#include "math/math_defs.hpp"
#include "math/shapes/AABB.hpp"
#include "scene/transform.hpp"
namespace emp {
class ColliderSystem;

struct CollisionInfo {
    Entity collider_entity;
    Entity collidee_entity;
    vec2f relative_velocity;
    float penetration;
    float normal_lagrange;
    vec2f collision_point;
    vec2f collision_normal;
    vec2f collider_radius;
    vec2f collidee_radius;
};
struct Collider {
    typedef std::vector<vec2f> ConvexVertexCloud;
    typedef std::function<void(const CollisionInfo&)>  CallbackFunc;
private:
    void m_updateNewTransform(const Transform& trans);
    AABB m_calcAABB() const;
    AABB m_aabb;

    static LayerMask collision_matrix[MAX_LAYERS];
    std::vector< std::pair<Entity, CallbackFunc> > m_callbacks;
public:
    void listen(Entity listener, CallbackFunc callback);
    void broadcastCollision(const CollisionInfo&);

    static void disableCollision(Layer layer1, Layer layer2);
    static void eableCollision(Layer layer1, Layer layer2);
    static bool canCollide(Layer layer1, Layer layer2);
    Layer collider_layer = 0;
    // potentially concave
    std::vector<vec2f> model_outline;
    std::vector<ConvexVertexCloud> model_shape;

    std::vector<vec2f> transformed_outline;
    std::vector<ConvexVertexCloud> transformed_shape;

    AABB aabb() const {
        return m_aabb;
    }
    Collider() {
    }
    Collider(std::vector<vec2f> shape, bool correctCOM = false);
    friend ColliderSystem;
};
// system for updating transfomred collider shapes
class ColliderSystem : public System<Transform, Collider> {
public:
    void update();
    void updateInstant(const Entity e);
};
}; // namespace emp
#endif
