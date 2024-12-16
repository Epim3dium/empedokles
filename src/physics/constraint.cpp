#include "constraint.hpp"
#include "core/coordinator.hpp"
#include "debug/log.hpp"
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
        inertia2 = INFINITY;
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
            vec2f offset_from_anchor,
            bool affect_anchor_offset_by_rotation ,
            vec2f pinch_point_rotated 
) {
    Constraint result;
    result.type = eConstraintType::PointAnchor;
    result.entity_list = {anchor, rigidbody};
    result.disabled_collision_between_bodies = true;


    result.point_anchor.anchor_affected_by_rotation= affect_anchor_offset_by_rotation;
    result.point_anchor.relative_position = offset_from_anchor;
    result.point_anchor.pinch_point_model =
            rotate(pinch_point_rotated, -rigid_trans->rotation);

    return result;
}
void Constraint::m_solvePointAnchor(float delta_time, Coordinator& ECS) {
    Entity anchor_entity = entity_list[0];
    Entity dynamic_entity = entity_list[1];
    assert(ECS.hasComponent<Transform>(anchor_entity));
    assert(ECS.hasComponent<Transform>(dynamic_entity));
    assert(ECS.hasComponent<Rigidbody>(dynamic_entity));

    auto target = point_anchor.relative_position;
    const auto& anchor_trans = *ECS.getComponent<Transform>(anchor_entity);
    auto& dynamic_trans = *ECS.getComponent<Transform>(dynamic_entity);
    auto& rigidbody = *ECS.getComponent<Rigidbody>(dynamic_entity);

    const vec2f& pos1 = anchor_trans.position;
    const vec2f& pos2 = dynamic_trans.position;
    auto anchor_point = pos1;
    if(point_anchor.anchor_affected_by_rotation){
        anchor_point -= rotateVec(point_anchor.relative_position, anchor_trans.rotation);
    }else {
        anchor_point -= point_anchor.relative_position;
    }
    auto dynamic_pinch = rotate(point_anchor.pinch_point_model, dynamic_trans.rotation);
    auto dynamic_point = (pos2 + dynamic_pinch);
    auto diff = dynamic_point - anchor_point ;
    auto norm = normal(diff);
    auto c = length(diff);
    if(nearlyEqual(c, 0.f))
        return;

    auto correction = calcPositionalCorrection(
            PositionalCorrectionInfo(
                    norm,
                    dynamic_entity,
                    dynamic_pinch,
                    ECS.getComponent<Rigidbody>(dynamic_entity),
                    anchor_entity,
                    vec2f(0),
                    nullptr
            ),
            c,
            norm,
            delta_time,
            compliance
    );
    dynamic_trans.position += correction.pos1_correction;
    dynamic_trans.rotation += correction.rot1_correction;
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

