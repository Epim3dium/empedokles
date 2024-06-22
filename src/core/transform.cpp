#include "transform.hpp"

namespace emp {
    void Transform::m_updateLocalTransform() {
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
    void Transform::addChild(Transform* child) {
        assert(child->m_parent == nullptr);
        m_children.push_back(child);
        child->m_parent = this;
    }
    void Transform::removeChild(Transform* child) {
        auto itr = std::find(m_children.begin(), m_children.end(), child);
        if(itr != m_children.end()) {
            child->m_parent = nullptr;
            m_children.erase(itr);
        }
    }
    void Transform::setParent(Transform* parent) {
        if(m_parent != nullptr) {
            m_parent->removeChild(this);
        }
        parent->addChild(this);
    }
    void Transform::update() {
        m_updateLocalTransform();
        if(m_parent != nullptr) {
            m_global_transform = m_parent->globalTransform();
            m_global_transform.combine(m_local_transform);
        }else {
            m_global_transform = m_local_transform;
        }
    }
};
