#include "physics_system.hpp"
#include "core/coordinator.hpp"
#include "physics/constraint.hpp"
namespace emp {
float PhysicsSystem::m_calcRestitution(
        float coef,
        float normal_speed,
        float pre_solve_norm_speed,
        vec2f gravity,
        float delT
) {
    // TODO: The XPBD paper has this, but it seems to be prevent bounces in
    // cases where bodies should clearly be bouncing. Maybe change the threshold
    // to be even lower? Or is this even needed at all?
    /*
    // If normal velocity is small enough, use restitution of 0 to avoid
    jittering
    */
    if (abs(normal_speed) <= gravity.length() * delT) {
        coef = 0.0;
    }
    return fmin(-normal_speed + (-coef * pre_solve_norm_speed), 0.f);
}
float PhysicsSystem::m_calcDynamicFriction(
        float coef,
        float tangent_speed,
        float generalized_inv_mass_sum,
        float normal_lagrange,
        float sub_dt
) {
    auto normal_impulse = normal_lagrange / sub_dt;
    return fmin(
            -(coef * abs(normal_impulse)),
            (tangent_speed / generalized_inv_mass_sum)
    );
}
vec2f PhysicsSystem::m_calcContactVel(vec2f vel, float ang_vel, vec2f r) {
    return vel + ang_vel * vec2f(-r.y, r.x);
}
PhysicsSystem::PenetrationConstraint PhysicsSystem::m_handleCollision(
        Entity e1,
        const int convexIdx1,
        Entity e2,
        const int convexIdx2,
        float delT,
        float compliance
) {
    PenetrationConstraint result;
    result.info.collider_entity = e1;
    result.info.collidee_entity = e2;

    auto& trans1 = getComponent<Transform>(e1);
    auto& rb1 = getComponent<Rigidbody>(e1);
    auto& col1 = getComponent<Collider>(e1);
    auto& mat1 = getComponent<Material>(e1);

    auto& trans2 = getComponent<Transform>(e2);
    auto& rb2 = getComponent<Rigidbody>(e2);
    auto& col2 = getComponent<Collider>(e2);
    auto& mat2 = getComponent<Material>(e2);

    // if(!Collider::canCollide(col1.collider_layer, col2.collider_layer)) {
    //     return {false};
    // }

    vec2f& pos1 = trans1.position;
    vec2f& pos2 = trans2.position;
    float& rot1 = trans1.rotation;
    float& rot2 = trans2.rotation;
    const float mass1 = rb1.mass();
    const float mass2 = rb2.mass();
    const float inertia1 = rb1.inertia();
    const float inertia2 = rb2.inertia();

    if (rb1.isStatic && rb2.isStatic)
        return result;
    result.isStatic1 = rb1.isStatic;
    result.isStatic2 = rb2.isStatic;

    const float sfriction =
            0.5f * (mat1.static_friction + mat2.static_friction);
    const float dfriction =
            0.5f * (mat1.dynamic_friction + mat2.dynamic_friction);
    const float restitution = 0.5f * (mat1.restitution + mat2.restitution);

    result.sfriction = sfriction;
    result.dfriction = dfriction;
    result.restitution = restitution;

    const auto& intersectingShape1 = col1.transformed_shape()[convexIdx1];
    const auto& intersectingShape2 = col2.transformed_shape()[convexIdx2];
    auto intersection =
            intersectPolygonPolygon(intersectingShape1, intersectingShape2);
    result.detected = intersection.detected;
    if (!intersection.detected) {
        return result;
    }
    float vn = 0.f;
    auto normal = -intersection.contact_normal;

    const auto penetration = intersection.overlap;
    auto p1 = intersection.cp1;
    auto p2 = intersection.cp2;
    auto d = dot(p2 - p1, normal);
    // TODO fix contact points
    //  if(d > penetration * 2.f) {
    //      EMP_LOG_DEBUG << "fixed";
    //      return {false};
    //  }
    EMP_DEBUGCALL(debug_contactpoints.push_back(p1));
    EMP_DEBUGCALL(debug_contactpoints.push_back(p2));

    result.info.collision_normal = intersection.contact_normal;
    result.info.penetration = penetration;

    // result.vel1_pre_solve = b1.vel;
    // result.vel2_pre_solve = b2.vel;
    // result.ang_vel1_pre_solve = b1.ang_vel;
    // result.ang_vel2_pre_solve = b2.ang_vel;

    const auto center_to_col_point1 = p1 - pos1;
    const auto center_to_col_point2 = p2 - pos2;
    const auto radius1 = rotateVec(center_to_col_point1, -rot1);
    const auto radius2 = rotateVec(center_to_col_point2, -rot2);
    result.info.collider_radius = radius1;
    result.info.collidee_radius = radius2;

    auto positional_correction = calcPositionalCorrection(
            PositionalCorrectionInfo(
                    normal,
                    e1,
                    center_to_col_point1,
                    &rb1,
                    e2,
                    center_to_col_point2,
                    &rb2
            ),
            penetration,
            normal,
            delT
    );
    float delta_lagrange = positional_correction.delta_lagrange;
    pos1 += positional_correction.pos1_correction;
    rot1 += positional_correction.rot1_correction;
    pos2 += positional_correction.pos2_correction;
    rot2 += positional_correction.rot2_correction;


    result.info.normal_lagrange = delta_lagrange;
    const auto normal_impulse = delta_lagrange / delT;

    auto delta_p1 = pos1 - rb1.previous_position() + rotateVec(radius1, rot1) -
                    rotateVec(radius1, rb1.previous_rotation());
    auto delta_p2 = pos2 - rb2.previous_position() + rotateVec(radius2, rot2) -
                    rotateVec(radius2, rb2.previous_rotation());
    auto delta_p = delta_p1 - delta_p2;
    auto delta_p_tangent = delta_p - dot(delta_p, normal) * normal;
    auto sliding_len = length(delta_p_tangent);

    if (sliding_len <= 0.f) {
        return result;
    }
    auto tangent = delta_p_tangent / sliding_len;
    if (sliding_len < sfriction * penetration) {
        positional_correction = calcPositionalCorrection(
                PositionalCorrectionInfo(
                        tangent,
                        e1,
                        center_to_col_point1,
                        &rb1,
                        e2,
                        center_to_col_point2,
                        &rb2
                ),
                sliding_len,
                tangent,
                delT,
                0.01f
        );
        pos1 += positional_correction.pos1_correction;
        rot1 += positional_correction.rot1_correction;
        pos2 += positional_correction.pos2_correction;
        rot2 += positional_correction.rot2_correction;

        delta_lagrange = positional_correction.delta_lagrange;
    }
    trans1.syncWithChange();
    trans2.syncWithChange();
    return result;
}
std::vector<CollidingPair> PhysicsSystem::m_broadPhase() {
    return SweepBroadPhase().findPotentialPairs(
            entities.begin(), entities.end()
    );
}
std::vector<PhysicsSystem::PenetrationConstraint> PhysicsSystem::m_narrowPhase(
        ColliderSystem& col_sys,
        const std::vector<CollidingPair>& pairs,
        float delT
) {
    std::vector<PenetrationConstraint> result;
    for (const auto [e1, e2, s1i, s2i] : pairs) {
        auto res = m_handleCollision(e1, s1i, e2, s2i, delT);
        if (!res.detected) {
            continue;
        }
        col_sys.notifyOfCollision(e1, e2, res.info);

        if(!res.isStatic1) {
            col_sys.updateInstant(e1);
        }
        if(!res.isStatic2) {
            col_sys.updateInstant(e2);
        }
        if(!res.isStatic1 && !res.isStatic2) {
            m_have_collided.set(e1);
            m_have_collided.set(e2);
            m_collision_islands.merge(e1, e2);
        }
        result.push_back(res);
    }
    return result;
}
void PhysicsSystem::m_broadcastCollisionMessages(
        const std::vector<PenetrationConstraint>& constraints
) {
    for(const auto& constraint : constraints) {
        auto col_info = constraint.info;
        auto& col1 = getComponent<Collider>(col_info.collider_entity);
        auto& col2 = getComponent<Collider>(col_info.collidee_entity);
        // col1.broadcastCollision(col_info);

        col_info.collision_normal *= -1.f;
        col_info.relative_velocity*= -1.f;
        std::swap(col_info.collider_entity, col_info.collidee_entity);
        std::swap(col_info.collider_radius, col_info.collidee_radius);
        // col2.broadcastCollision(col_info);
    }
}
// need to update colliders after
void PhysicsSystem::m_solveVelocities(
        std::vector<PenetrationConstraint>& constraints, float delT
) {
    for (auto& constraint : constraints) {
        const auto e1 = constraint.info.collider_entity;
        const auto e2 = constraint.info.collidee_entity;

        const auto& trans1 = getComponent<Transform>(e1);
        auto& rb1 = getComponent<Rigidbody>(e1);

        const auto& trans2 = getComponent<Transform>(e2);
        auto& rb2 = getComponent<Rigidbody>(e2);

        const auto restitution = constraint.restitution;

        const auto r1model = rotateVec(constraint.info.collider_radius, trans1.rotation);
        const auto r2model = rotateVec(constraint.info.collidee_radius, trans2.rotation);

        const auto pre_solve_contact_vel1 = m_calcContactVel(
                rb1.previous_velocity() , rb1.previous_angular_velocity(), r1model
        );
        const auto pre_solve_contact_vel2 = m_calcContactVel(
                rb2.previous_velocity(), rb2.previous_angular_velocity(), r2model
        );
        const auto pre_solve_relative_vel =
                pre_solve_contact_vel1 - pre_solve_contact_vel2;
        const auto pre_solve_normal_speed =
                dot(pre_solve_relative_vel, constraint.info.collision_normal);

        const auto contact_vel1 =
                m_calcContactVel(rb1.velocity, rb1.angular_velocity, r1model);
        const auto contact_vel2 =
                m_calcContactVel(rb2.velocity, rb2.angular_velocity, r2model);
        const auto relative_vel = contact_vel1 - contact_vel2;
        constraint.info.relative_velocity = relative_vel;
        const auto normal_speed = dot(relative_vel, constraint.info.collision_normal);

        const auto tangent_vel =
                relative_vel - constraint.info.collision_normal * normal_speed;
        const auto tangent_speed = length(tangent_vel);

        vec2f p = {0, 0};
        auto restitution_speed = m_calcRestitution(
                restitution,
                normal_speed,
                pre_solve_normal_speed,
                {0.f, gravity},
                delT
        );
        if (abs(restitution_speed) > 0.f) {
            const auto w1 =
                    rb1.generalizedInverseMass(r1model, constraint.info.collision_normal);
            const auto w2 =
                    rb2.generalizedInverseMass(r2model, constraint.info.collision_normal);
            const auto restitution_impulse = restitution_speed / (w1 + w2);
            p += restitution_impulse * constraint.info.collision_normal;
        }

        const float sfriction = constraint.sfriction;
        const float dfriction = constraint.dfriction;
        // Compute dynamic friction
        if (abs(tangent_speed) > 0.f) {
            const auto tangent = tangent_vel / tangent_speed;
            const auto w1 = rb1.generalizedInverseMass(r1model, tangent);
            const auto w2 = rb2.generalizedInverseMass(r2model, tangent);
            const auto friction_impulse = m_calcDynamicFriction(
                    dfriction,
                    tangent_speed,
                    w1 + w2,
                    constraint.info.normal_lagrange,
                    delT
            );
            p += friction_impulse * tangent;
            // constraint.contact.tangent_impulse += friction_impulse;
        }
        if (!rb1.isStatic) {
            const auto delta_lin_vel = p / rb1.mass();
            const auto delta_ang_vel = perp_dot(r1model, p) / rb1.inertia();
            rb1.velocity += delta_lin_vel;
            rb1.angular_velocity += delta_ang_vel;
        }
        if (!rb2.isStatic) {
            const auto delta_lin_vel = p / rb2.mass();
            const auto delta_ang_vel = perp_dot(r2model, p) / rb2.inertia();
            rb2.velocity -= delta_lin_vel;
            rb2.angular_velocity -= delta_ang_vel;
        }
    }
}
void PhysicsSystem::m_applyGravity() {
    for (const auto e : entities) {
        if(m_isDormant(e)) {
            continue;
        }
        auto& rb = getComponent<Rigidbody>(e);
        if (!rb.isStatic) {
            rb.force += vec2f(0, gravity) * rb.mass();
        }
    }
}
void PhysicsSystem::m_applyAirDrag() {
    for (const auto e : entities) {
        if(m_isDormant(e)) {
            continue;
        }
        auto& rb = getComponent<Rigidbody>(e);
        auto& material = getComponent<Material>(e);
        auto magnitude = dot(rb.velocity, rb.velocity);
        if(magnitude == 0.f) {
            continue;
        }
        auto direction = -normal(rb.velocity);
        if (!rb.isStatic) {
            rb.force += direction * magnitude * material.air_friction;
        }
    }
}
void PhysicsSystem::m_processSleep(float delta_time) {
    for (const auto e : entities) {
        auto& rb = getComponent<Rigidbody>(e);
        if(rb.isStatic)
            continue;
        auto head = m_collision_islands.group(e);
        if(length(rb.velocity) > SLOW_VEL) {
            m_have_been_slow_for[head] = 0.f;
        }
        if(m_have_been_slow_for[head] == 0.f) {
            m_have_been_slow_for[e] = 0.f;
            m_collision_islands.isolate(e);
        }
    }
    for(const auto e : entities) {
        auto& rb = getComponent<Rigidbody>(e);
        if(rb.isStatic)
            continue;
        if(m_collision_islands.isHead(e)) {
            m_have_been_slow_for[e] += delta_time;
        }
    }
    for (const auto e : entities) {
        auto& rb = getComponent<Rigidbody>(e);
        auto& col = getComponent<Collider>(e);
        if(rb.isStatic)
            continue;
        if(m_isDormant(e)) {
            rb.isSleeping = true;
            col.isNonMoving = true;
        }else {
            rb.isSleeping = false;
            col.isNonMoving = false;
        }
    }
}
bool PhysicsSystem::m_isDormant(Entity e) {
    return m_have_been_slow_for[m_collision_islands.group(e)] > DORMANT_TIME;
}
void PhysicsSystem::m_step(
        TransformSystem& trans_sys,
        ColliderSystem& col_sys,
        RigidbodySystem& rb_sys,
        ConstraintSystem& const_sys,
        float delta_time
) {
    m_applyGravity();
    m_applyAirDrag();
    m_processSleep(delta_time);
    rb_sys.integrate(delta_time);
    trans_sys.update();
    col_sys.update();
    const_sys.update(delta_time);
    auto potential_pairs = m_broadPhase();
    auto penetrations = m_narrowPhase(col_sys, potential_pairs, delta_time);
    
    trans_sys.update();
    rb_sys.deriveVelocities(delta_time);
    m_solveVelocities(penetrations, delta_time);
    m_broadcastCollisionMessages(penetrations);
}
void PhysicsSystem::update(
        TransformSystem& trans_sys,
        ColliderSystem& col_sys,
        RigidbodySystem& rb_sys,
        ConstraintSystem& const_sys,
        float delT
) {
    EMP_DEBUGCALL(debug_contactpoints.clear();)
    m_have_collided.reset();
    for (int i = 0; i < substep_count; i++) {
        m_step(trans_sys,
               col_sys,
               rb_sys,
               const_sys,
               delT / (float)substep_count);
        for (const auto e : entities) {
            auto& rb = getComponent<Rigidbody>(e);
            rb.force = {0, 0};
            rb.torque = 0.f;
        }
    }
    col_sys.processCollisionNotifications();
    m_separateNonColliding();
}
void PhysicsSystem::m_separateNonColliding() {
    for(int e = 0; e < MAX_ENTITIES; e++) {
        if(!m_have_collided.test(e) && !m_isDormant(e)) {
            m_collision_islands.isolate(e);
        }
    }
}
void PhysicsSystem::onEntityAdded(Entity entity) {
}
}; // namespace emp

