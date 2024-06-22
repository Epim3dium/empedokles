#include "rigidbody.hpp"
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
void Rigidbody::update() {
    m_mass = m_collider->area() * m_density;
    m_inertia = m_collider->inertiaDevMass() * m_density;
}
};
