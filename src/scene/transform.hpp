#ifndef EMP_TRANSFORM_HPP
#define EMP_TRANSFORM_HPP
#include "core/component_system.hpp"
#include "math/math_defs.hpp"
#include "SFML/Graphics/Transform.hpp"
#include <vector>

namespace emp {
class Transform;
typedef ComponentSystem<Transform> TransformSystem;
typedef ComponentInstance<Transform, TransformSystem> TransformInstance;
class Transform {
    sf::Transform m_local_transform;
    sf::Transform m_global_transform;
    emp::Transform* m_parent = nullptr;
    std::vector<emp::Transform*> m_children;


    void m_updateLocalTransform();
    void m_syncWithChange();
public:
    vec2f position = vec2f(0.f, 0.f);
    float rotation = 0.f;
    vec2f scale = vec2f(0.f, 0.f);

    void setPositionNow(vec2f p) ;
    void setRotationNow(float r) ;
    void setScaleNow(vec2f s) ;
    inline const sf::Transform& localTransform() const {
        return m_local_transform;
    }
    inline const sf::Transform& globalTransform() const {
        return m_global_transform;
    }

    void addChild(Transform* child);
    void removeChild(Transform* child);
    void setParent(Transform* parent);
    void update();
    Transform(const Transform&) = delete;
    Transform(Transform&&) = delete;
    Transform& operator=(const Transform&) = delete;
private:
    friend TransformSystem;
    Transform(vec2f pos, float rot = 0.f, vec2f s = {1.f, 1.f}) : position(pos), rotation(rot), scale(s) { update(); }
};
};
#endif
