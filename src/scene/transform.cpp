#include "transform.hpp"
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace emp {
void Transform::m_updateLocalTransform() {
    m_local_transform = TransformMatrix(1.f);
    m_local_transform = glm::translate(
            m_local_transform, glm::vec3(position.x, position.y, 0.f)
    );
    m_local_transform =
            glm::rotate(m_local_transform, rotation, vec3f(0.f, 0.f, 1.f));
    m_local_transform =
            glm::scale(m_local_transform, vec3f(scale.x, scale.y, 1.f));
}
void Transform::syncWithChange() {
    m_updateLocalTransform();
    m_global_transform = m_parents_global_transform * m_local_transform;
}
void Transform::setPositionNow(vec2f p) {
    position = p;
    syncWithChange();
}
void Transform::setRotationNow(float r) {
    rotation = r;
    syncWithChange();
}
void Transform::setScaleNow(vec2f s) {
    scale = s;
    syncWithChange();
}
void TransformSystem::update() {
    for (auto entity : entities) {
        auto& trans = getComponent<Transform>(entity);
        trans.m_parents_global_transform = glm::mat4x4(1.f);
        trans.m_updateLocalTransform();
        trans.m_global_transform =
                trans.m_parents_global_transform * trans.m_local_transform;
    }
}
}; // namespace emp
