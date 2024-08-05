#include "transform.hpp"
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace emp {
    void Transform::m_updateLocalTransform() {
        m_local_transform = TransformMatrix(1.f);
        m_local_transform = glm::translate(m_local_transform, glm::vec3(position.x, position.y, 0.f));
        m_local_transform = glm::rotate(m_local_transform, rotation, vec3f(0.f, 0.f, 1.f));
        m_local_transform = glm::scale(m_local_transform, vec3f(scale.x, scale.y, 1.f));
    }
    void Transform::m_syncWithChange() {
        m_global_transform = m_global_transform * glm::inverse(m_local_transform);
        m_updateLocalTransform();
        m_global_transform = m_global_transform * m_local_transform;
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
            auto& trans = getComponent<Transform>(entity);
            trans.m_updateLocalTransform();
            trans.m_global_transform = trans.m_local_transform;
        }
    }
};
