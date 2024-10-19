#include "collider.hpp"
#include "core/coordinator.hpp"
#include "debug/debug.hpp"
#include "math/geometry_func.hpp"
namespace emp {
LayerMask Collider::collision_matrix[MAX_LAYERS];

void Collider::listen(Entity listener, CallbackFunc callback) {
    m_callbacks.push_back({listener, callback});
}
void Collider::broadcastCollision(const CollisionInfo& info) {
    for(int i = 0; i < m_callbacks.size(); i++) {
        if(!coordinator.isEntityAlive(m_callbacks[i].first)) {
            m_callbacks.erase(m_callbacks.begin() + i);
            i--;
            continue;
        }
        m_callbacks[i].second(info);
    }
}
bool Collider::canCollide(Layer layer1, Layer layer2) {
    LayerMask* mat = Collider::collision_matrix;
    auto lesser = std::min(layer1, layer2);
    auto major= std::max(layer1, layer2);
    return mat[lesser].test(major) == false; 
}
void Collider::disableCollision(Layer layer1, Layer layer2) {
    LayerMask* mat = Collider::collision_matrix;
    auto lesser = std::min(layer1, layer2);
    auto major= std::max(layer1, layer2);
    mat[lesser] = mat[lesser] | LayerMask().set(major, true); 
}
void Collider::eableCollision(Layer layer1, Layer layer2) {
    LayerMask* mat = Collider::collision_matrix;
    auto lesser = std::min(layer1, layer2);
    auto major= std::max(layer1, layer2);

    mat[lesser] = mat[lesser] & LayerMask().set().set(major, false); 
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
    for (const auto& p : transformed_outline) {
        result.expandToContain(p);
    }
    return result;
}
void Collider::m_updateNewTransform(const Transform& transform) {
    for (int i = 0; i < model_shape.size(); i++) {
        auto& poly = model_shape[i];
        for (int ii = 0; ii < poly.size(); ii++) {
            transformed_shape[i][ii] =
                    transformPoint(transform.global(), poly[ii]);
        }
        auto center = std::reduce(transformed_shape[i].begin(), transformed_shape[i].end()) / static_cast<float>(poly.size());
        std::sort(transformed_shape[i].begin(), transformed_shape[i].end(), [&](vec2f a, vec2f b) {
            return atan2(a.y - center.y, a.x - center.x) > atan2(b.y - center.y, b.x - center.x);
        });
    }
    for (int i = 0; i < model_outline.size(); i++) {
        transformed_outline[i] =
                transformPoint(transform.global(), model_outline[i]);
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
    model_outline = shape;
    auto MIA = calculateMassInertiaArea(model_outline);
    if (correctCOM) {
        for (auto& p : model_outline) {
            p -= MIA.centroid;
        }
    }
    transformed_outline = model_outline;

    auto triangles = triangulateAsVector(model_outline);
    model_shape = mergeToConvex(triangles);
    transformed_shape = model_shape;
}
}; // namespace emp
