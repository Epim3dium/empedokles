#include "rigidbody.hpp"
#include "math/math_func.hpp"
namespace emp {
Rigidbody::Rigidbody(Transform* transform, Collider* collider, float density) {
    m_collider = collider;
    m_transform = transform;
    m_density = density;
    m_mass = collider->area() * density;
    m_inertia = collider->inertiaDevMass() * density;
    prev_pos = transform->position;
    prev_rot = transform->rotation;
}
#define SQ(x) ((x) * (x))
float Rigidbody::generalizedInverseMass(vec2f radius, vec2f normal) const {
    if(isStatic)
        return 0.f;
    return 1.f / mass() + (SQ(cross(radius, normal)) / inertia());
}
void Rigidbody::update() {
    m_mass = m_collider->area() * m_density;
    m_inertia = m_collider->inertiaDevMass() * m_density;
}
void Rigidbody::integrate(float delT) {
    prev_pos = m_transform->position;
    vel += delT * force / mass();
    m_transform->position += vel * delT;

    prev_rot = m_transform->rotation;
    ang_vel += delT * torque / inertia();
    m_transform->rotation += ang_vel * delT;
}
void Rigidbody::deriveVelocities(float delT) {
    if(isStatic)
        return;
    vel_pre_solve = vel;
    vel = (m_transform->position - prev_pos) / delT;
    ang_vel_pre_solve = ang_vel;
    ang_vel = (m_transform->rotation - prev_rot) / delT;
}
};
