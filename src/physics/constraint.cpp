#include "constraint.hpp"
#include "physics_system.hpp"
namespace emp {
    Constraint Constraint::createPointAnchor(Entity anchor, Entity rigidbody) {
        assert(coordinator.hasComponent<Transform>(anchor));
        assert(coordinator.hasComponent<Transform>(rigidbody));
        assert(coordinator.hasComponent<Rigidbody>(rigidbody));

        Constraint result;
        result.type = eConstraintType::PointAnchor;
        result.entity_list = {anchor, rigidbody};
        result.disabled_collision_between_bodies = true;

        const auto& anchor_trans = *coordinator.getComponent<Transform>(anchor);
        const auto& rigid_trans = *coordinator.getComponent<Transform>(rigidbody);

        result.point_anchor.relative_position = anchor_trans.position - rigid_trans.position;

        return result;
    }
    void Constraint::m_solvePointAnchor(float delta_time) {
        Entity anchor = entity_list[0];
        Entity rigidbody= entity_list[1];
        assert(coordinator.hasComponent<Transform>(anchor));
        assert(coordinator.hasComponent<Transform>(rigidbody));
        assert(coordinator.hasComponent<Rigidbody>(rigidbody));

        auto target = point_anchor.relative_position;
        const auto& anchor_trans = *coordinator.getComponent<Transform>(anchor);
        auto& rigid_trans = *coordinator.getComponent<Transform>(rigidbody);
        auto relative_position = anchor_trans.position - rigid_trans.position;

        auto& rb = *coordinator.getComponent<Rigidbody>(rigidbody);

        const vec2f& pos1 = anchor_trans.position;
        const vec2f& pos2 = rigid_trans.position;
        auto diff = pos1-pos2;
        const float& rot2 = anchor_trans.rotation;
        const float mass1 = INFINITY;
        const float mass2 = rb.mass();
        const float inertia1 = 0.f;
        const float inertia2 = rb.inertia();

        const auto r1 = vec2f(0, 0);
        const auto r2 = rotateVec(target, rigid_trans.rotation);
        const auto w1 = 0.f;
        const auto w2 = rb.generalizedInverseMass(r2, normal(diff));

        const auto tilde_compliance = 0.f / (delta_time * delta_time);

        auto delta_lagrange = -length(diff);
        delta_lagrange /= (w1 + w2 + tilde_compliance);

        auto p = delta_lagrange * normal(diff);

        if(!rb.isStatic) {
            rigid_trans.setPositionNow(pos2 - p / mass2);
            rigid_trans.setRotationNow(rot2 - cross(r2, p) / inertia2);
        }
    }
    void Constraint::solve(float delta_time) {
        switch(type) {
            case emp::eConstraintType::PointAnchor:
                m_solvePointAnchor(delta_time);
                break;
            case emp::eConstraintType::Undefined:
                assert(false);
                break;
        }

    }
};
