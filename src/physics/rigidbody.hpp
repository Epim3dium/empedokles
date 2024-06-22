#ifndef EMP_RIGIDBODY_HPP
#define EMP_RIGIDBODY_HPP
#include "core/transform.hpp"
#include "math/math_defs.hpp"
#include "physics/collider.hpp"
namespace emp {
class Rigidbody {
    float m_inertia = 1.f;
    float m_mass = 1.f;
    Collider* m_collider;
    Transform* m_transform;
    float m_density = 1.f;
public:
    float inertia() const {
        return m_inertia;
    }
    float mass() const {
        return m_mass;
    }

    vec2f prev_pos = vec2f(0.f, 0.f);
    vec2f vel_pre_solve = vec2f(0.f, 0.f);
    vec2f vel = vec2f(0.f, 0.f);
    vec2f force = vec2f(0.f, 0.f);

    float prev_rot = 0.f;
    float ang_vel_pre_solve = 0.f;
    float ang_vel = 0.f;
    float torque = 0.f;

    bool isStatic = false;

    void update();
    Rigidbody(Transform* transform, Collider* collider, float density = 1.f);
};
};
#endif
