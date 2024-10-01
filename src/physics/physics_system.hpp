#ifndef EMP_PHYSICS_SYSTEM_HPP
#define EMP_PHYSICS_SYSTEM_HPP
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "physics/broad_phase.hpp"
#include "physics/constraint.hpp"
#include "physics/collider.hpp"
#include "physics/material.hpp"
#include "physics/rigidbody.hpp"
#include "debug/debug.hpp"

#include <memory>
namespace emp {
    struct Constraint;
class PhysicsSystem : public System<Transform, Collider, Rigidbody, Material> {
    struct PenetrationConstraint {
        bool detected = false;
        Entity entity1;
        Entity entity2;
        vec2f normal;
        float penetration;
        vec2f pos1_pre_col;
        vec2f pos2_pre_col;
        float rot1_pre_col;
        float rot2_pre_col;
        //not rotated not translated (model space)
        vec2f radius1;
        //not rotated not translated (model space)
        vec2f radius2;
        float normal_lagrange;
    };
    float m_calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT);
    float m_calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange, float sub_dt);
    struct PositionalCorrectionInfo {
        Entity entity1;
        vec2f radius1;
        bool isStatic1;
        float inertia1;
        float mass1;
        float generalized_inverse_mass1;
        Entity entity2;
        vec2f radius2;
        bool isStatic2;
        float inertia2;
        float mass2;
        float generalized_inverse_mass2;
        PositionalCorrectionInfo() {}
        explicit PositionalCorrectionInfo(vec2f normal, Entity e1, vec2f r1, Entity e2, vec2f r2) : entity1(e1), radius1(r1), entity2(e2), radius2(r2) 
        {
            assert(coordinator.hasComponent<Rigidbody>(e1));
            auto& rb1 = *coordinator.getComponent<Rigidbody>(e1);
            isStatic1 = rb1.isStatic;
            inertia1 = rb1.inertia();
            mass1 = rb1.mass();
            generalized_inverse_mass1 = rb1.generalizedInverseMass(r1, normal);

            if(coordinator.hasComponent<Rigidbody>(e2)) {
                auto& rb2 = *coordinator.getComponent<Rigidbody>(e2);
                isStatic2 = rb2.isStatic;
                inertia2 = rb2.inertia();
                mass2 = rb2.mass();
                generalized_inverse_mass2 = rb2.generalizedInverseMass(r2, normal);
            }else {
                isStatic2 = true;
                inertia2 = 0.f;
                mass2 = INFINITY;
                generalized_inverse_mass2 = 0.f;
            }
        }
    };
    static float m_applyPositionalCorrection(PositionalCorrectionInfo info, float c, vec2f normal, float delT, float compliance = 0.f);
    vec2f m_calcContactVel(vec2f vel, float ang_vel, vec2f r);

    PenetrationConstraint m_handleCollision(Entity b1, const int convexIdx1, Entity b2, const int convexIdx2, float delT, float compliance = 0.f);
    
    std::vector<CollidingPair> m_broadPhase();
    std::vector<PenetrationConstraint> m_narrowPhase(ColliderSystem& col_sys, const std::vector<CollidingPair>& pairs, float delT);
    //need to update colliders after
    void m_solveVelocities(std::vector<PenetrationConstraint>& constraints, float delT);
    void m_step(TransformSystem& trans_sys, ColliderSystem& col_sys, RigidbodySystem& rb_sys,ConstraintSystem& const_sys, float deltaTime);
public:
    EMP_DEBUGCALL(std::vector<vec2f> debug_contactpoints);
    float gravity = 1.f;
    size_t substep_count = 8U;
    void update(TransformSystem& trans_sys, ColliderSystem& col_sys, RigidbodySystem& rb_sys, ConstraintSystem& const_sys,
            float delT);
    void onEntityAdded(Entity entity) override;
    friend Constraint;
};
};
#endif
