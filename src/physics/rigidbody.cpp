#include "rigidbody.hpp"
#include "core/coordinator.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
namespace emp {
#define SQ(x) ((x) * (x))
float Rigidbody::generalizedInverseMass(vec2f radius, vec2f normal) const {
    if(isStatic)
        return 0.f;
    return 1.f / mass() + (SQ(cross(radius, normal)) / inertia());
}
void RigidbodySystem::updateMasses() {
    for(auto entity : entities) {
        auto& rigidobdy = coordinator.getComponent<Rigidbody>(entity);
        coordinator.getComponent<Collider>(entity);
        if(rigidobdy.useAutomaticMass && coordinator.hasComponent<Collider>(entity)) {
            const auto& collider = coordinator.getComponent<Collider>(entity);
            rigidobdy.real_mass = collider.area * rigidobdy.real_density;
            rigidobdy.real_inertia = collider.inertia_dev_mass * rigidobdy.real_density;
        }
    }
}
void RigidbodySystem::integrate(float delT) {
    for(auto entity : entities) {
        auto& rigidbody = coordinator.getComponent<Rigidbody>(entity);
        auto& transform = coordinator.getComponent<Transform2D>(entity);
        rigidbody.prev_pos = transform.position;
        rigidbody.vel += delT * rigidbody.force / rigidbody.mass();
        transform.position += rigidbody.vel * delT;

        rigidbody.prev_rot = transform.rotation;
        rigidbody.ang_vel += delT * rigidbody.torque / rigidbody.inertia();
        transform.rotation += rigidbody.ang_vel * delT;
    }
}
void RigidbodySystem::deriveVelocities(float delT) {
    for(auto entity : entities) {
        auto& rigidbody = coordinator.getComponent<Rigidbody>(entity);
        auto& transform = coordinator.getComponent<Transform2D>(entity);
        if(rigidbody.isStatic)
            continue;
        rigidbody.vel_pre_solve = rigidbody.vel;
        rigidbody.vel = (transform.position - rigidbody.prev_pos) / delT;
        rigidbody.ang_vel_pre_solve = rigidbody.ang_vel;
        rigidbody.ang_vel = (transform.rotation - rigidbody.prev_rot) / delT;
    }
}
};
