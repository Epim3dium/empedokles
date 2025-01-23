#include "collider.hpp"
#include <random>
#include "core/coordinator.hpp"
#include "debug/debug.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
namespace emp {

void CollisionInfo::flip() {
    std::swap(collider_entity, collidee_entity);
    std::swap(collider_radius, collidee_radius);
    collision_normal *= -1.f;
    relative_velocity *= -1.f;
}

ColliderSystem& ColliderSystem::onCollisionEnter(
        Entity listener, Entity target, CollisionEnterCallback&& func
) {
    this->m_enter_callbacks[target].push_back(std::move(func));
    return *this;
}
ColliderSystem& ColliderSystem::onCollisionExit(
        Entity listener, Entity target, CollisionExitCallback&& func
) {
    this->m_exit_callbacks[target].push_back(std::move(func));
    return *this;
}
void ColliderSystem::notifyOfCollision(Entity a, Entity b, CollisionInfo col_info) {
    FitIntoOne hasher;
    hasher.a = std::min(a, b);
    hasher.b = std::max(a, b);
    m_collisions_occured_this_frame[hasher.hash] = col_info;
}

void ColliderSystem::processCollisionNotifications() {
    auto& this_frame = m_collisions_occured_this_frame;
    auto& last_frame = m_collisions_occured_last_frame;

    std::unordered_set<uint64_t> new_events;
    std::unordered_set<uint64_t> this_frame_collisions;
    for(auto it = this_frame.begin(); it != this_frame.end(); it++) {
        this_frame_collisions.insert(it->first);
        if(last_frame.contains(it->first)) {
            //deleting all collisions that are still happening
            last_frame.erase(it->first);
        }else {
            //keeping track of new collisions
            new_events.insert(it->first);
        }
    }
    //by deleting all collisions that are still happening we are left with collisions that just ended
    auto& ended_events = last_frame;
    for(auto& ended : ended_events) {
        FitIntoOne dehasher;
        dehasher.hash = ended;
        auto isAsleeping = ECS().isEntityAlive(dehasher.a) &&
            getComponent<Collider>(dehasher.a).isNonMoving;
        auto isBsleeping = ECS().isEntityAlive(dehasher.b) &&
            getComponent<Collider>(dehasher.b).isNonMoving;
        if(isAsleeping && isBsleeping) {
            this_frame_collisions.insert(dehasher.hash);
            continue;
        }

        callAllOnExitCallbacksFor(dehasher.a, dehasher.b);
        callAllOnExitCallbacksFor(dehasher.b, dehasher.a);
    }
    for(auto& event : new_events) {
        FitIntoOne dehasher;
        dehasher.hash = event;
        auto& info = m_collisions_occured_this_frame.at(event);
        assert(info.collider_entity == dehasher.a || info.collider_entity == dehasher.b);
        assert(info.collidee_entity == dehasher.a || info.collidee_entity == dehasher.b);

        callAllOnEnterCallbacksFor(info.collider_entity, info);
        info.flip();
        callAllOnEnterCallbacksFor(info.collider_entity, info);
    }

    m_collisions_occured_last_frame = this_frame_collisions;
    m_collisions_occured_this_frame.clear();
}
void ColliderSystem::callAllOnEnterCallbacksFor(Entity e, const CollisionInfo& info) {
    if(m_enter_callbacks.contains(e)) {
        auto& callbacks = m_enter_callbacks.at(e);
        for(auto callback : callbacks) {
            callback(info);
        }
    }
}
void ColliderSystem::callAllOnExitCallbacksFor(Entity e, Entity other) {
    if(m_exit_callbacks.contains(e)) {
        auto& callbacks = m_exit_callbacks.at(e);
        for(auto callback : callbacks) {
            callback(e, other);
        }
    }
}
void notifyOfCollision(Entity a, Entity b);

ColliderSystem::ColliderSystem() {
    for(auto& layer : collision_matrix) {
        layer.set();
    }
}
bool ColliderSystem::canCollide(Layer layer1, Layer layer2) const {
    auto lesser = std::min(layer1, layer2);
    auto major= std::max(layer1, layer2);
    return collision_matrix[lesser].test(major) == true; 
}
void ColliderSystem::disableCollision(Layer layer1, Layer layer2) {
    auto lesser = std::min(layer1, layer2);
    auto major= std::max(layer1, layer2);
    collision_matrix[lesser] = collision_matrix[lesser] & LayerMask().set().set(major, false); 
}
void ColliderSystem::eableCollision(Layer layer1, Layer layer2) {
    auto lesser = std::min(layer1, layer2);
    auto major= std::max(layer1, layer2);
    collision_matrix[lesser] = collision_matrix[lesser] | LayerMask().set(major, true); 
}
// Collider::Collider(std::vector<vec2f> shape, emp::Transform* trans, bool
// correctCOM) {
//     auto MIA = calculateMassInertiaArea(shape);
//     inertia_dev_mass = MIA.MMOI;
//     area = MIA.area;
//     if(correctCOM) {
//         for(auto& p : shape) {
//             p -= MIA.centroid;
//         }
//     }
//     model_outline = shape;
//     transformed_outline = model_outline;
//
//     auto triangles = triangulateAsVector(shape);
//     model_shape = mergeToConvex(triangles);
//     transformed_shape = model_shape;
//     m_transform = trans;
// }
AABB Collider::m_calcAABB() const {
    auto result = AABB::Expandable();
    for (const auto& p : m_transformed_outline) {
        result.expandToContain(p);
    }
    return result;
}
void Collider::m_updateNewTransform(const Transform& transform) {
    for (int i = 0; i < m_model_shape.size(); i++) {
        auto& poly = m_model_shape[i];
        for (int ii = 0; ii < poly.size(); ii++) {
            m_transformed_shape[i][ii] =
                    transformPoint(transform.global(), poly[ii]);
        }
        auto center = std::reduce(m_transformed_shape[i].begin(),
                          m_transformed_shape[i].end()) /
                      static_cast<float>(poly.size());
        std::sort(m_transformed_shape[i].begin(),
            m_transformed_shape[i].end(),
            [&](vec2f a, vec2f b) {
                return atan2(a.y - center.y, a.x - center.x) >
                       atan2(b.y - center.y, b.x - center.x);
            });
    }
    for (int i = 0; i < m_model_outline.size(); i++) {
        m_transformed_outline[i] =
                transformPoint(transform.global(), m_model_outline[i]);
    }
    m_aabb = m_calcAABB();
}
void ColliderSystem::update() {
    for (auto entity : entities) {
        auto& transform = getComponent<Transform>(entity);
        auto& collider = getComponent<Collider>(entity);
        collider.m_updateNewTransform(transform);
    }
}
void ColliderSystem::updateInstant(const Entity entity) {
    assert(entities.contains(entity) && "system must contain that entity");
    auto& transform = getComponent<Transform>(entity);
    auto& collider = getComponent<Collider>(entity);
    collider.m_updateNewTransform(transform);
}
Collider::Collider(std::vector<vec2f> shape, bool correctCOM) {
    m_model_outline = shape;
    auto MIA = calculateMassInertiaArea(m_model_outline);
    if (correctCOM) {
        for (auto& p : m_model_outline) {
            p -= MIA.centroid;
        }
    }
    m_transformed_outline = m_model_outline;

    auto triangles = triangulateAsVector(m_model_outline);
    m_model_shape = mergeToConvex(triangles);
    m_transformed_shape = m_model_shape;
    m_aabb = m_calcAABB();
}
void ColliderSystem::onEntityRemoved(Entity entity) {
    m_exit_callbacks.erase(entity);
    m_enter_callbacks.erase(entity);
}
}; // namespace emp
