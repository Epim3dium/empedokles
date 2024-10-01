#include "constraint.hpp"
#include "physics_system.hpp"
namespace emp {
    Constraint Constraint::createPointAnchor(Entity anchor, Entity rigidbody, vec2f pinch_point_rotated ) {
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
        result.point_anchor.pinch_point_model = rotate(pinch_point_rotated, -rigid_trans.rotation);
        auto rp = result.point_anchor.relative_position;
        EMP_LOG_DEBUG << rp.x << "\t" << rp.y << "\n";

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
        auto& rb = *coordinator.getComponent<Rigidbody>(rigidbody);

        const vec2f& pos1 = anchor_trans.position;
        const vec2f& pos2 = rigid_trans.position;
        auto diff = pos1 - point_anchor.relative_position - (pos2 + rotate(point_anchor.pinch_point_model, rigid_trans.rotation));
        auto norm = normal(diff);
        auto c = length(diff);

        auto damping_force = dot(rb.vel, norm) * damping * delta_time;
        if(length(rb.vel) == 0.f) {
            damping_force = 0.f;
        }
        PhysicsSystem::m_applyPositionalCorrection(
            PhysicsSystem::PositionalCorrectionInfo(norm, rigidbody, point_anchor.pinch_point_model, anchor, vec2f(0, 0)),
            c - damping_force, -norm, delta_time, (1.f / stiffness));
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
    void ConstraintSystem::update(float delta_time) {
        for(auto e : entities) {
            getComponent<Constraint>(e).solve(delta_time);
        }
    }
};
