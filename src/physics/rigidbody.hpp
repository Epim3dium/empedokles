#ifndef EMP_RIGIDBODY_HPP
#define EMP_RIGIDBODY_HPP
#include "math/math_defs.hpp"
#include "scene/transform.hpp"
namespace emp {
struct RigidbodySystem;
class Rigidbody {
    float real_inertia = 1.f;
    float real_mass = 1.f;
    float real_density = 1.f;

    vec2f prev_pos = vec2f(0.f, 0.f);
    vec2f velocity_pre_solve = vec2f(0.f, 0.f);

    float prev_rot = 0.f;
    float ang_velocity_pre_solve = 0.f;
public:
    inline vec2f previous_position() const {
        return prev_pos;
    }
    inline float previous_rotation() const {
        return prev_rot;
    }
    inline vec2f previous_velocity() const {
        return velocity_pre_solve;
    }
    inline float previous_angular_velocity() const {
        return ang_velocity_pre_solve;
    }
    bool isStatic = false;
    bool isRotationLocked = false;

    bool isSleeping = false;
    bool useAutomaticMass = true;

    vec2f velocity = vec2f(0.f, 0.f);
    vec2f force = vec2f(0.f, 0.f);
    float angular_velocity = 0.f;
    float torque = 0.f;

    float inertia() const {
        return (isStatic || isRotationLocked) ? INFINITY : real_inertia;
    }
    float mass() const {
        return isStatic ? INFINITY : real_mass;
    }
    float generalizedInverseMass(vec2f radius, vec2f normal) const;

    Rigidbody(bool is_static = false, bool is_rot_locked = false, bool use_automatic_mass = true, float density = 1.f);
    friend RigidbodySystem;
};
class RigidbodySystem : public System<Transform, Rigidbody> {
public:
    void integrate(float delT);
    void deriveVelocities(float delT);
    void updateMasses();
};
}; // namespace emp
#endif
