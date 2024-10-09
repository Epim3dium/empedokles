#include "constraint.hpp"
#include "physics_system.hpp"
namespace emp {
    PositionalCorrectionInfo::PositionalCorrectionInfo(vec2f normal, Entity e1, vec2f r1, Entity e2, vec2f r2) : entity1(e1), radius1(r1), entity2(e2), radius2(r2) 
    {
        assert(coordinator.hasComponent<Rigidbody>(e1));
        auto& rb1 = *coordinator.getComponent<Rigidbody>(e1);
        isStatic1 = rb1.isStatic;
        inertia1 = rb1.inertia();
        mass1 = rb1.mass();
        generalized_inverse_mass1 = rb1.generalizedInverseMass(r1, normal);

        if(coordinator.hasComponent<Rigidbody>(e2)) {
            auto& rb2 = *coordinator.getComponent<Rigidbody>(e2);
            isStatic2 = rb2.isStatic;
            inertia2 = rb2.inertia();
            mass2 = rb2.mass();
            generalized_inverse_mass2 = rb2.generalizedInverseMass(r2, normal);
        }else {
            isStatic2 = true;
            inertia2 = 0.f;
            mass2 = INFINITY;
            generalized_inverse_mass2 = 0.f;
        }
    }
    float applyPositionalCorrection(PositionalCorrectionInfo info, float c, vec2f normal, float delT, float compliance) {
        auto e1 = info.entity1;
        auto e2 = info.entity2;
        auto& trans1 = *coordinator.getComponent<Transform>(e1);

        auto& trans2 = *coordinator.getComponent<Transform>(e2);

        const vec2f& pos1 = trans1.position;
        const vec2f& pos2 = trans2.position;
        const float& rot1 = trans1.rotation;
        const float& rot2 = trans2.rotation;

        const auto r1 = rotateVec(info.radius1, trans1.rotation);
        const auto r2 = rotateVec(info.radius2, trans2.rotation);
        const auto w1 = info.generalized_inverse_mass1;
        const auto w2 = info.generalized_inverse_mass2;

        const auto tilde_compliance = compliance / (delT * delT);

        auto delta_lagrange = -c;
        delta_lagrange /= (w1 + w2 + tilde_compliance);

        auto p = delta_lagrange * normal;

        if(!info.isStatic1) {
            trans1.position += p / info.mass1;
            trans1.rotation += cross(r1, p) / info.inertia1;
            trans1.syncWithChange();
        }
        if(!info.isStatic2) {
            trans2.position += -p / info.mass2;
            trans2.rotation += -cross(r2, p) / info.inertia2;
            trans2.syncWithChange();
        }

        return delta_lagrange;
    }
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
        applyPositionalCorrection(
            PositionalCorrectionInfo(norm, rigidbody, point_anchor.pinch_point_model, anchor, vec2f(0, 0)),
            c - damping_force, -norm, delta_time, compliance);
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
