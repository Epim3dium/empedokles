#ifndef EMP_PHYSICS_SYSTEM_HPP
#define EMP_PHYSICS_SYSTEM_HPP
#include "math/geometry_func.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
#include "physics/material.hpp"
#include "physics/rigidbody.hpp"
namespace emp {
class PhysicsSystem {
    struct Entry {
        Transform* transform;
        Collider* collider;
        Rigidbody* rigidbody;
        Material* material;
    };
    std::vector<Entry> m_entries;
    struct PenetrationConstraint {
        bool detected = false;
        Entry body1;
        Entry body2;
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
    float m_applyPositionalCorrection(Entry& b1, Entry& b2, float c, vec2f normal, vec2f radius1, vec2f radius2, float delT, float compliance = 0.f);
    vec2f m_calcContactVel(vec2f vel, float ang_vel, vec2f r);
    PenetrationConstraint m_handleCollision(Entry b1, const int convexIdx1, Entry b2, const int convexIdx2, float delT, float compliance = 0.f);
    std::vector<PenetrationConstraint> m_detectPenetrations(std::vector<Entry>& bodies, float delT);
    //need to update colliders after
    void m_integrate(std::vector<Entry>& bodies, float h);
    //need to update colliders after
    void m_deriveVelocities(std::vector<Entry>& bodies, float h);
    void m_solveVelocities(std::vector<PenetrationConstraint>& constraints, float delT);
    void m_step(std::vector<Entry> bodies, float deltaTime);
public:
    void add(Transform* trans, Collider* col, Rigidbody* rb, Material* mat) {
        m_entries.push_back({trans, col, rb, mat});
    }
    std::vector<vec2f> contactpoints;
    void substeps(float delT, float gravity, size_t substepCount = 8U);
};
};
#endif
