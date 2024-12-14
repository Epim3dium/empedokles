#include "constraint.hpp"
#include "core/coordinator.hpp"
#include "math/math_func.hpp"
#include "physics/rigidbody.hpp"
#include "physics_system.hpp"
namespace emp {
PositionalCorrectionInfo::PositionalCorrectionInfo(
        vec2f normal,
        Entity e1,
        vec2f center_to_col1,
        const Rigidbody* rb1,
        Entity e2,
        vec2f center_to_col2,
        const Rigidbody* rb2
)
    : entity1(e1),
      center_to_collision1(center_to_col1),
      entity2(e2),
      center_to_collision2(center_to_col2) {
    if (rb1 == nullptr) {
        EMP_LOG(WARNING) << "no rigidbody to create PositionalCorrectionInfo";
    }

    isStatic1 = rb1->isStatic;
    inertia1 = rb1->inertia();
    mass1 = rb1->mass();
    generalized_inverse_mass1 =
            rb1->generalizedInverseMass(center_to_col1, normal);

    if (rb2 != nullptr) {
        isStatic2 = rb2->isStatic;
        inertia2 = rb2->inertia();
        mass2 = rb2->mass();
        generalized_inverse_mass2 =
                rb2->generalizedInverseMass(center_to_col2, normal);
    } else {
        isStatic2 = true;
        inertia2 = 0.f;
        mass2 = INFINITY;
        generalized_inverse_mass2 = 0.f;
    }
}
PositionalCorrResult calcPositionalCorrection(
        PositionalCorrectionInfo info,
        float c,
        vec2f normal,
        float delT,
        float compliance
) {
    PositionalCorrResult result;
    if(nearlyEqual(c, 0.f)) {
        return result;
    }

    const auto w1 = info.generalized_inverse_mass1;
    const auto w2 = info.generalized_inverse_mass2;

    const auto tilde_compliance = compliance / (delT * delT);

    auto delta_lagrange = -c;
    delta_lagrange /= (w1 + w2 + tilde_compliance);

    auto p = delta_lagrange * normal;

    if (!info.isStatic1) {
        result.pos1_correction = p / info.mass1;
        result.rot1_correction =
                perp_dot(info.center_to_collision1, p) / info.inertia1;
    }
    if (!info.isStatic2) {
        result.pos2_correction = -p / info.mass2;
        result.rot2_correction =
                -perp_dot(info.center_to_collision2, p) / info.inertia2;
    }
    result.delta_lagrange = delta_lagrange;

    return result;
}
Constraint Constraint::createPointAnchor(
        Entity anchor, const Transform* anchor_trans,
        Entity rigidbody,const Transform* rigid_trans,
        vec2f pinch_point_rotated
) {
    Constraint result;
    result.type = eConstraintType::PointAnchor;
    result.entity_list = {anchor, rigidbody};
    result.disabled_collision_between_bodies = true;


    result.point_anchor.relative_position =
            anchor_trans->position - rigid_trans->position;
    result.point_anchor.pinch_point_model =
            rotate(pinch_point_rotated, -rigid_trans->rotation);
    auto rp = result.point_anchor.relative_position;

    return result;
}
void Constraint::m_solvePointAnchor(float delta_time, Coordinator& ECS) {
    Entity anchor = entity_list[0];
    Entity rigidbody = entity_list[1];
    assert(ECS.hasComponent<Transform>(anchor));
    assert(ECS.hasComponent<Transform>(rigidbody));
    assert(ECS.hasComponent<Rigidbody>(rigidbody));

    auto target = point_anchor.relative_position;
    const auto& anchor_trans = *ECS.getComponent<Transform>(anchor);
    auto& rigid_trans = *ECS.getComponent<Transform>(rigidbody);
    auto& rb = *ECS.getComponent<Rigidbody>(rigidbody);

    const vec2f& pos1 = anchor_trans.position;
    const vec2f& pos2 = rigid_trans.position;
    auto diff = pos1 - point_anchor.relative_position -
                (pos2 +
                 rotate(point_anchor.pinch_point_model, rigid_trans.rotation));
    auto norm = normal(diff);
    auto c = length(diff);

    auto damping_force = dot(rb.velocity, norm) * damping * delta_time;
    if (length(rb.velocity) == 0.f) {
        damping_force = 0.f;
    }
    calcPositionalCorrection(
            PositionalCorrectionInfo(
                    norm,
                    rigidbody,
                    point_anchor.pinch_point_model,
                    ECS.getComponent<Rigidbody>(rigidbody),
                    anchor,
                    vec2f(0, 0),
                    ECS.getComponent<Rigidbody>(anchor)
            ),
            c - damping_force,
            -norm,
            delta_time,
            compliance
    );
}
void Constraint::solve(float delta_time, Coordinator& ECS) {
    switch (type) {
    case emp::eConstraintType::PointAnchor:
        m_solvePointAnchor(delta_time, ECS);
        break;
    case emp::eConstraintType::Undefined:
        assert(false);
        break;
    }
}
void ConstraintSystem::update(float delta_time) {
    for (auto e : entities) {
        getComponent<Constraint>(e).solve(delta_time, ECS());
    }
}
}; // namespace emp

