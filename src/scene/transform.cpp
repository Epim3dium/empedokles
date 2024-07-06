#include "transform.hpp"
#include "core/coordinator.hpp"
#include "debug/log.hpp"

namespace emp {
    void Transform::m_updateLocalTransform() {
        m_local_transform = sf::Transform::Identity;
        m_local_transform.translate(position);
        m_local_transform.rotate(rotation / M_PI * 180.f);
        m_local_transform.scale(scale);
    }
    void Transform::m_syncWithChange() {
        m_global_transform.combine(m_local_transform.getInverse());
        m_updateLocalTransform();
        m_global_transform.combine(m_local_transform);
    }
    void Transform::setPositionNow(vec2f p) {
        position = p;
        m_syncWithChange();
    }
    void Transform::setRotationNow(float r) {
        rotation = r;
        m_syncWithChange();
    }
    void Transform::setScaleNow(vec2f s) {
        scale = s;
        m_syncWithChange();
    }
    void TransformSystem::update() {
        for(auto entity : entities) {
            auto& trans = coordinator.getComponent<Transform>(entity);
            trans.m_updateLocalTransform();
            trans.m_global_transform = trans.m_local_transform;
        }
    }
};
