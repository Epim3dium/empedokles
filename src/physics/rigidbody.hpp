#ifndef EMP_RIGIDBODY_HPP
#define EMP_RIGIDBODY_HPP
#include "core/transform.hpp"
#include "math/math_defs.hpp"
#include "physics/collider.hpp"
namespace emp {
class Rigidbody;
typedef ComponentSystem<Rigidbody> RigidbodySystem;
typedef ComponentInstance<Rigidbody, RigidbodySystem> RigidbodyInstance;
class Rigidbody {
    float m_inertia = 1.f;
    float m_mass = 1.f;
    Collider* m_collider;
    Transform* m_transform;
    float m_density = 1.f;
public:
    float inertia() const {
        return isStatic ? INFINITY : m_inertia;
    }
    float mass() const {
        return isStatic ? INFINITY : m_mass;
    }
    float generalizedInverseMass(vec2f radius, vec2f normal) const;

    vec2f prev_pos = vec2f(0.f, 0.f);
    vec2f vel_pre_solve = vec2f(0.f, 0.f);
    vec2f vel = vec2f(0.f, 0.f);
    vec2f force = vec2f(0.f, 0.f);

    float prev_rot = 0.f;
    float ang_vel_pre_solve = 0.f;
    float ang_vel = 0.f;
    float torque = 0.f;

    bool isStatic = false;

    void integrate(float delT);
    void deriveVelocities(float delT);
    void update();
    Rigidbody(const Rigidbody&) = delete;
    Rigidbody(Rigidbody&&) = delete;
    Rigidbody& operator=(const Rigidbody&) = delete;
private:
    friend RigidbodySystem;
    Rigidbody(Transform* transform, Collider* collider, float density = 1.f);
};
};
#endif
