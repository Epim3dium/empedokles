#include "rigidbody.hpp"
#include "core/coordinator.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
namespace emp {
#define SQ(x) ((x) * (x))
float Rigidbody::generalizedInverseMass(vec2f radius, vec2f normal) const {
    if (isStatic)
        return 0.f;
    return 1.f / mass() + (SQ(perp_dot(radius, normal)) / inertia());
}
void RigidbodySystem::updateMasses() {
    for (auto entity : entities) {
        auto& rigidobdy = getComponent<Rigidbody>(entity);
        const auto collider = coordinator.getComponent<Collider>(entity);
        if (rigidobdy.useAutomaticMass && collider != nullptr) {
            rigidobdy.real_mass = collider->area * rigidobdy.real_density;
            rigidobdy.real_inertia =
                    collider->inertia_dev_mass * rigidobdy.real_density;
        }
    }
}
void RigidbodySystem::integrate(float delT) {
    for (auto entity : entities) {
        auto& rigidbody = getComponent<Rigidbody>(entity);
        auto& transform = getComponent<Transform>(entity);
        rigidbody.prev_pos = transform.position;
        rigidbody.velocity += delT * rigidbody.force / rigidbody.mass();
        transform.position += rigidbody.velocity * delT;

        rigidbody.prev_rot = transform.rotation;
        rigidbody.angular_velocity += delT * rigidbody.torque / rigidbody.inertia();
        transform.rotation += rigidbody.angular_velocity * delT;
    }
}
void RigidbodySystem::deriveVelocities(float delT) {
    for (auto entity : entities) {
        auto& rigidbody = getComponent<Rigidbody>(entity);
        auto& transform = getComponent<Transform>(entity);
        if (rigidbody.isStatic)
            continue;
        rigidbody.vel_pre_solve = rigidbody.velocity;
        rigidbody.velocity = (transform.position - rigidbody.prev_pos) / delT;
        rigidbody.ang_vel_pre_solve = rigidbody.angular_velocity;
        rigidbody.angular_velocity = (transform.rotation - rigidbody.prev_rot) / delT;
    }
}
}; // namespace emp
