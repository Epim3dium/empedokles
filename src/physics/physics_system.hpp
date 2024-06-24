#ifndef EMP_PHYSICS_SYSTEM_HPP
#define EMP_PHYSICS_SYSTEM_HPP
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "physics/broad_phase.hpp"
#include "physics/collider.hpp"
#include "physics/material.hpp"
#include "physics/rigidbody.hpp"
#include "debug/debug.hpp"

#include <memory>
namespace emp {
struct PhysicsManagerEntry {
    Transform* transform;
    Collider* collider;
    Rigidbody* rigidbody;
    Material* material;
};
class PhysicsSystem {
    std::vector<PhysicsManagerEntry> m_entries;
    std::unique_ptr<BroadPhaseSolver> m_broad_phase_solver;
    struct PenetrationConstraint {
        bool detected = false;
        PhysicsManagerEntry body1;
        PhysicsManagerEntry body2;
        vec2f normal;
        float penetration;
        vec2f pos1_at_col;
        vec2f pos2_at_col;
        float rot1_at_col;
        float rot2_at_col;
        //not rotated not translated
        vec2f radius1;
        //not rotated not translated
        vec2f radius2;
        float normal_lagrange;
    };
    float m_calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT);
    float m_calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange, float sub_dt);
    float m_applyPositionalCorrection(PhysicsManagerEntry& b1, PhysicsManagerEntry& b2, float c, vec2f normal, vec2f radius1, vec2f radius2, float delT, float compliance = 0.f);
    vec2f m_calcContactVel(vec2f vel, float ang_vel, vec2f r);
    PenetrationConstraint m_handleCollision(PhysicsManagerEntry b1, const int convexIdx1, PhysicsManagerEntry b2, const int convexIdx2, float delT, float compliance = 0.f);
    
    std::vector<BroadPhaseSolver::CollidingPair> m_broadPhase(std::vector<PhysicsManagerEntry>& bodies);
    std::vector<PenetrationConstraint> m_narrowPhase(const std::vector<BroadPhaseSolver::CollidingPair>& pairs, std::vector<PhysicsManagerEntry>& bodies, float delT);
    //need to update colliders after
    void m_integrate(std::vector<PhysicsManagerEntry>& bodies, float h);
    //need to update colliders after
    void m_deriveVelocities(std::vector<PhysicsManagerEntry>& bodies, float h);
    void m_solveVelocities(std::vector<PenetrationConstraint>& constraints, float delT);
    void m_step(std::vector<PhysicsManagerEntry> bodies, float deltaTime);
public:
    EMP_DEBUGCALL(std::vector<vec2f> debug_contactpoints);

    void add(Transform* trans, Collider* col, Rigidbody* rb, Material* mat) {
        m_entries.push_back({trans, col, rb, mat});
    }
    void substeps(float delT, float gravity, size_t substepCount = 8U);

    PhysicsSystem() : m_broad_phase_solver(std::make_unique<SweepBroadPhase>()) {}
};
};
#endif
