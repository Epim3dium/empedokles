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

        const vec2f& pos1 = anchor_trans.position;
        const vec2f& pos2 = rigid_trans.position;
        auto diff = pos1-pos2;
        PhysicsSystem::m_applyPositionalCorrection(
            PhysicsSystem::PositionalCorrectionInfo(normal(diff), rigidbody, vec2f(0, 0), anchor, vec2f(0, 0)),
            length(diff), -normal(diff), delta_time, (1.f / (stiffness * delta_time)));
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
