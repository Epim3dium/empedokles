#include "constraint.hpp"
#include <glm/ext/quaternion_common.hpp>
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
typedef Constraint::Builder Builder;
Builder& Builder::addConstrainedEntity(Entity entity, const Transform& transform) {
    entity_list.push_back({entity, &transform});
    return *this;
}
Builder& Builder::addAnchorEntity(Entity entity, const Transform& transform) {
    anchor = {entity, &transform};
    return *this;
}

Builder& Builder::setCompliance(float compliance_) {
    this->compliance = compliance_;
    return *this;
}
Builder& Builder::setDamping(float damping_) {
    this->damping = damping_;
    return *this;
}

Builder& Builder::enableCollision(bool enable ) {
    this->enabled_collision_between_bodies = enable;
    return *this;
}

Builder& Builder::setConnectionGlobalPoint(vec2f point) {
    assert(this->type == eConstraintType::Undefined && "trying to change constraint type");
    type = eConstraintType::SwivelPoint;
    global_point = point;
    return *this;
}
Builder& Builder::setConnectionRelativePoint(vec2f rel1, vec2f rel2) {
    assert(this->type == eConstraintType::Undefined && "trying to change constraint type");
    type = eConstraintType::SwivelPoint;
    point_rel1 = rel1;
    point_rel2 = rel2;
    return *this;
}

Constraint Builder::build() {
    assert(type != eConstraintType::Undefined && "too little information in constraint builder - no type");
    Constraint result;
    result.damping = damping;
    result.compliance = compliance;
    result.enabled_collision_between_bodies = this->enabled_collision_between_bodies;
    //conversion of constraint types if anchor set (static object)
    bool usingAnchor = anchor.first != -1 && anchor.second != nullptr;
    if(usingAnchor) {
        switch(type) {
            case eConstraintType::SwivelPoint:
                type = eConstraintType::SwivelPointAnchored;
            break;
            default:
            break;
        }
        entity_list.insert(entity_list.begin(), anchor);
    }
    for(auto [e, _] : entity_list) {
        result.entity_list.push_back(e);
    }
    result.type = type;
    switch(type) {
        case eConstraintType::SwivelPointAnchored:
            assert(usingAnchor);
            if(!glm::isnan(global_point.x)) {
                point_rel1 = global_point - anchor.second->position;
                point_rel1 = rotate(point_rel1, -anchor.second->rotation);
                point_rel2 = global_point - entity_list.back().second->position;
                point_rel2 = rotate(point_rel2, -entity_list.back().second->rotation);
            }
            result.swivel_anchored.anchor_point_model = point_rel1;
            result.swivel_anchored.pinch_point_model = point_rel2;
        break;
        case eConstraintType::SwivelPoint:
            assert(!usingAnchor && entity_list.size() == 2);
            if(!glm::isnan(global_point.x)) {
                point_rel1 = global_point - entity_list.front().second->position;
                point_rel1 = rotate(point_rel1, -entity_list.front().second->rotation);
                point_rel2 = global_point - entity_list.back().second->position;
                point_rel2 = rotate(point_rel2, -entity_list.back().second->rotation);
            }
            result.swivel_dynamic.pinch_point_model1 = point_rel1;
            result.swivel_dynamic.pinch_point_model2 = point_rel2;
        break;
        default:
        break;
    }
    return result;
}
void Constraint::m_solvePointSwivel(float delta_time, Coordinator& ECS) {
}
void Constraint::m_solvePointAnchor(float delta_time, Coordinator& ECS) {
    Entity anchor_entity = entity_list[0];
    Entity dynamic_entity = entity_list[1];
    assert(ECS.hasComponent<Transform>(anchor_entity));
    assert(ECS.hasComponent<Transform>(dynamic_entity));
    assert(ECS.hasComponent<Rigidbody>(dynamic_entity));

    auto target = swivel_anchored.anchor_point_model;
    const auto& anchor_trans = *ECS.getComponent<Transform>(anchor_entity);
    auto& dynamic_trans = *ECS.getComponent<Transform>(dynamic_entity);
    auto& rigidbody = *ECS.getComponent<Rigidbody>(dynamic_entity);

    const vec2f& pos1 = anchor_trans.position;
    const vec2f& pos2 = dynamic_trans.position;
    auto anchor_point = pos1 + rotate(swivel_anchored.anchor_point_model, anchor_trans.rotation);
    auto dynamic_pinch = rotate(swivel_anchored.pinch_point_model, dynamic_trans.rotation);
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
    case emp::eConstraintType::SwivelPointAnchored:
        m_solvePointAnchor(delta_time, ECS);
        break;
    case emp::eConstraintType::SwivelPoint:
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

