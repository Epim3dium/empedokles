#ifndef EMP_RIGIDBODY_HPP
#define EMP_RIGIDBODY_HPP
#include "math/math_defs.hpp"
#include "scene/transform.hpp"
namespace emp {
class Rigidbody;
class Rigidbody {
public:
    bool isStatic = false;

    float real_inertia = 1.f;
    float real_mass = 1.f;
    float real_density = 1.f;
    bool useAutomaticMass = true;
    float inertia() const {
        return isStatic ? INFINITY : real_inertia;
    }
    float mass() const {
        return isStatic ? INFINITY : real_mass;
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
};
class RigidbodySystem : public System<Transform, Rigidbody> {
public:
    void integrate(float delT);
    void deriveVelocities(float delT);
    void updateMasses();
};
}; // namespace emp
#endif
