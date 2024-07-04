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
    Collider::Collider(std::vector<vec2f> shape, bool correctCOM) {
        model_outline = shape;
        auto MIA = calculateMassInertiaArea(model_outline);
        inertia_dev_mass = MIA.MMOI;
        area = MIA.area;
        if(correctCOM) {
            for(auto& p : model_outline) {
                p -= MIA.centroid;
            }
        }
        transformed_outline = model_outline;

        auto triangles = triangulateAsVector(model_outline);
        model_shape = mergeToConvex(triangles);
        transformed_shape = model_shape;
    }
    void ColliderSystem::onEntityAdded(Entity entity) {
        auto& transform = coordinator.getComponent<Transform>(entity);
        auto& collider = coordinator.getComponent<Collider>(entity);

        EMP_LOG(DEBUG2) << "added collider to ColliderSystem";
    }
};
