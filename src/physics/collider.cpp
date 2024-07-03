#include "collider.hpp"
#include "core/coordinator.hpp"
#include "math/geometry_func.hpp"
#include "debug/debug.hpp"
namespace emp {
    // Collider::Collider(std::vector<vec2f> shape, emp::Transform* trans, bool correctCOM) {
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
    void ColliderSystem::update() {
        for(auto entity : entities) {
            auto& transform = coordinator.getComponent<Transform>(entity);
            auto& collider = coordinator.getComponent<Collider>(entity);
            for(int i = 0; i < collider.model_shape.size(); i++) {
                auto& poly = collider.model_shape[i];
                for(int ii = 0; ii < poly.size(); ii++) {
                    collider.transformed_shape[i][ii] = transform.globalTransform().transformPoint(poly[ii]);
                }
            }
            for (int i = 0; i < collider.model_outline.size(); i++) {
                collider.transformed_outline[i] = transform.globalTransform().transformPoint(collider.model_outline[i]);
            }
        }
    }
#define EMP_CORRECT_COM true
    void ColliderSystem::onEntityAdded(Entity entity) {
        auto& transform = coordinator.getComponent<Transform>(entity);
        auto& collider = coordinator.getComponent<Collider>(entity);

        EMP_LOG(DEBUG2) << "added collider to ColliderSystem";
        auto MIA = calculateMassInertiaArea(collider.model_outline);
        collider.inertia_dev_mass = MIA.MMOI;
        collider.area = MIA.area;
        if(EMP_CORRECT_COM) {
            for(auto& p : collider.model_outline) {
                p -= MIA.centroid;
            }
        }
        collider.transformed_outline = collider.model_outline;

        auto triangles = triangulateAsVector(collider.model_outline);
        collider.model_shape = mergeToConvex(triangles);
        collider.transformed_shape = collider.model_shape;
    }
};
