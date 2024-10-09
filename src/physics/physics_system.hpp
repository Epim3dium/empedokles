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
        float rot1_pre_col;
        float rot2_pre_col;
        //not rotated not translated (model space)
        vec2f radius1;
        //not rotated not translated (model space)
        vec2f radius2;
        float normal_lagrange;
        float sfriction;
        float dfriction;
        float restitution;
    };
    float m_calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT);
    float m_calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange, float sub_dt);
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
