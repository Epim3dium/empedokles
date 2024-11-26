#include "transform.hpp"
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "debug/debug.hpp"
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
EMP_DEBUGCALL(
    bool updated_entities[MAX_ENTITIES]{0};
)
    std::stack<Entity> to_process;
    to_process.push(Coordinator::world());

    while(!to_process.empty()) {
        auto entity = to_process.top();
        to_process.pop();
        auto& transform = getComponent<Transform>(entity);
        transform.m_updateLocalTransform();
        transform.m_global_transform =
                transform.m_parents_global_transform * transform.m_local_transform;

        for(const auto child : transform.children()) {
            auto& childs_trans = getComponent<Transform>(child);
            childs_trans.m_parents_global_transform = transform.global();
            to_process.push(child);
        }
EMP_DEBUGCALL(
        updated_entities[entity] = true;
)
    }

EMP_DEBUGCALL(
        for(auto e : entities) {
            if(!updated_entities[e]) {
                EMP_LOG(WARNING) << "didn't updatede transform: " << e
                                 << ", because of invalid parent";
            }
        }
)
    // for (auto entity : entities) {
    //     auto& trans = getComponent<Transform>(entity);
    //     trans.m_parents_global_transform = glm::mat4x4(1.f);
    //     trans.m_updateLocalTransform();
    //     trans.m_global_transform =
    //             trans.m_parents_global_transform * trans.m_local_transform;
    // }
}
    void TransformSystem::onEntityAdded(Entity entity) {
        if(entity == Coordinator::world())
            return;
        auto& transform = getComponent<Transform>(entity);

        const auto parent = transform.parent();
        auto parent_transform = ECS().getComponent<Transform>(parent);

        if(parent_transform == nullptr) {
            EMP_LOG(WARNING) << "assigned a parent without transform, reassigning to world";
            transform.m_parent_entity = Coordinator::world();
            parent_transform = ECS().getComponent<Transform>(Coordinator::world());
        }
        parent_transform->m_children_entities.push_back(entity);
    }
    void TransformSystem::onEntityRemoved(Entity entity) {
        auto& transform = getComponent<Transform>(entity);
        const auto parent = transform.parent();
        auto parent_transform = ECS().getComponent<Transform>(parent);
        if(parent_transform == nullptr) {
            EMP_LOG(WARNING) << "parent without transfrom, but had when assigning";
            return;
        }
        auto& children = parent_transform->m_children_entities;
        auto itr = std::find(children.begin(), children.end(), entity);
        if(itr != children.end()) {
            children.erase(itr);
        }
    }
}; // namespace emp
