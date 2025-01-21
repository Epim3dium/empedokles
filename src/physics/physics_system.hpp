#ifndef EMP_PHYSICS_SYSTEM_HPP
#define EMP_PHYSICS_SYSTEM_HPP
#include "debug/debug.hpp"
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "physics/broad_phase.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/material.hpp"
#include "physics/rigidbody.hpp"
#include "templates/disjoint_set.hpp"

#include <memory>
namespace emp {
struct Constraint;
class PhysicsSystem : public System<Transform, Collider, Rigidbody, Material> {
    struct PenetrationConstraint {
        bool detected = false;
        CollisionInfo info;
        // not rotated not translated (model space)
        bool isStatic1;
        bool isStatic2;
        float sfriction;
        float dfriction;
        float restitution;
    };
    float m_calcRestitution(
            float coef,
            float normal_speed,
            float pre_solve_norm_speed,
            vec2f gravity,
            float delT
    );
    float m_calcDynamicFriction(
            float coef,
            float tangent_speed,
            float generalized_inv_mass_sum,
            float normal_lagrange,
            float sub_dt
    );
    vec2f m_calcContactVel(vec2f vel, float ang_vel, vec2f r);

    PenetrationConstraint m_handleCollision(
            Entity b1,
            const int convexIdx1,
            Entity b2,
            const int convexIdx2,
            float delT,
            float compliance = 0.f
    );

    std::vector<CollidingPair> m_broadPhase();
    std::vector<PenetrationConstraint> m_narrowPhase(
            ColliderSystem& col_sys,
            const std::vector<CollidingPair>& pairs,
            float delT
    );
    // need to update colliders after
    void m_solveVelocities(
            std::vector<PenetrationConstraint>& constraints, float delT
    );
    void m_broadcastCollisionMessages(
            const std::vector<PenetrationConstraint>& constraints
    );
    void m_applyGravity();
    void m_applyAirDrag();
    void m_processSleep(float delta_time, ConstraintSystem& constr_sys);
    void m_separateNonColliding();
    void m_step(
            TransformSystem& trans_sys,
            ColliderSystem& col_sys,
            RigidbodySystem& rb_sys,
            ConstraintSystem& const_sys,
            float deltaTime
    );

public:
    DisjointSet<MAX_ENTITIES> m_collision_islands;
    std::bitset<MAX_ENTITIES> m_have_collided;
    bool useDeactivation = true;
    static constexpr float SLOW_VEL = 15.f;
    static constexpr float DORMANT_TIME = 3.f;
    float m_have_been_slow_for[MAX_ENTITIES] {0.f};
    bool m_isDormant(Entity e);


    vec2f gravity = {0.f, 1.f};
    size_t substep_count = 8U;
    void update(
            TransformSystem& trans_sys,
            ColliderSystem& col_sys,
            RigidbodySystem& rb_sys,
            ConstraintSystem& const_sys,
            float delT
    );
    void onEntityAdded(Entity entity) override;
    friend Constraint;
};
}; // namespace emp
#endif
