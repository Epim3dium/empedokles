#include "physics_system.hpp"
#include "core/coordinator.hpp"
#include "physics/constraint.hpp"
namespace emp {
    float PhysicsSystem::m_calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT) {
        // TODO: The XPBD paper has this, but it seems to be prevent bounces in cases
        // where bodies should clearly be bouncing.
        // Maybe change the threshold to be even lower? Or is this even needed at all?
        /*
        // If normal velocity is small enough, use restitution of 0 to avoid jittering
        */
        if( abs(normal_speed) <= gravity.length() * delT) {
            coef = 0.0;
        }
        return fmin(-normal_speed + (-coef * pre_solve_norm_speed), 0.f);
    }
    float PhysicsSystem::m_calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange, float sub_dt) {
        auto normal_impulse = normal_lagrange / sub_dt;
        return fmin(-(coef * abs(normal_impulse)), (tangent_speed / generalized_inv_mass_sum));
    }
    vec2f PhysicsSystem::m_calcContactVel(vec2f vel, float ang_vel, vec2f r) {
        return vel + ang_vel * vec2f(-r.y, r.x);
    }
    PhysicsSystem::PenetrationConstraint PhysicsSystem::m_handleCollision(Entity e1, const int convexIdx1, Entity e2, const int convexIdx2, float delT, float compliance) {
        PenetrationConstraint result;
        result.entity1 = e1;
        result.entity2 = e2;

        auto& trans1 = getComponent<Transform>(e1);
        auto& rb1 = getComponent<Rigidbody>(e1);
        auto& col1 = getComponent<Collider>(e1);
        auto& mat1 = getComponent<Material>(e1);

        auto& trans2 = getComponent<Transform>(e2);
        auto& rb2 = getComponent<Rigidbody>(e2);
        auto& col2 = getComponent<Collider>(e2);
        auto& mat2 = getComponent<Material>(e2);

        const vec2f& pos1 = trans1.position;
        const vec2f& pos2 = trans2.position;
        const float& rot1 = trans1.rotation;
        const float& rot2 = trans2.rotation;
        const float mass1 = rb1.mass();
        const float mass2 = rb2.mass();
        const float inertia1 = rb1.inertia();
        const float inertia2 = rb2.inertia();
        
        if(rb1.isStatic && rb2.isStatic)
            return result;
        const float sfriction = 0.5f * (mat1.static_friction  + mat2.static_friction);
        const float dfriction = 0.5f * (mat1.dynamic_friction + mat2.dynamic_friction);
        const float restitution = 0.5f * (mat1.restitution + mat2.restitution);

        result.sfriction = sfriction;
        result.dfriction = dfriction;
        result.restitution= restitution;

        const auto& intersectingShape1 = col1.transformed_shape[convexIdx1];
        const auto& intersectingShape2 = col2.transformed_shape[convexIdx2];
        auto intersection = intersectPolygonPolygon(intersectingShape1, intersectingShape2);
        result.detected = intersection.detected;
        if(!intersection.detected) {
            return result;
        }
        float vn = 0.f;
        auto normal = -intersection.contact_normal;

        const auto penetration = intersection.overlap;
        auto p1 = intersection.cp1;
        auto p2 = intersection.cp2;
        auto d = dot(p2 - p1, normal);
        //TODO fix contact points
        // if(d > penetration * 2.f) {
        //     EMP_LOG_DEBUG << "fixed";
        //     return {false};
        // }
        EMP_DEBUGCALL(debug_contactpoints.push_back(p1));
        EMP_DEBUGCALL(debug_contactpoints.push_back(p2));

        result.normal = intersection.contact_normal;
        result.penetration = penetration;
        result.rot1_pre_col = rot1;
        result.rot2_pre_col = rot2;

        // result.vel1_pre_solve = b1.vel;
        // result.vel2_pre_solve = b2.vel;
        // result.ang_vel1_pre_solve = b1.ang_vel;
        // result.ang_vel2_pre_solve = b2.ang_vel;

        const auto r1modeled = p1 - pos1;
        const auto r2modeled = p2 - pos2;
        const auto r1 = rotateVec(r1modeled, -rot1);
        const auto r2 = rotateVec(r2modeled, -rot2);
        result.radius1 = r1;
        result.radius2 = r2;

        auto delta_lagrange = applyPositionalCorrection(PositionalCorrectionInfo(normal, e1, r1, &rb1, e2, r2, &rb2), penetration, normal, delT);
        result.normal_lagrange = delta_lagrange;
        const auto normal_impulse = delta_lagrange / delT;

        auto delta_p1 = pos1 - rb1.prev_pos
            + rotateVec(r1, rot1)
            - rotateVec(r1, rb1.prev_rot);
        auto delta_p2 = pos2 - rb2.prev_pos
            + rotateVec(r2, rot2)
            - rotateVec(r2, rb2.prev_rot);
        auto delta_p = delta_p1 - delta_p2;
        auto delta_p_tangent = delta_p - dot(delta_p, normal) * normal;
        auto sliding_len = length(delta_p_tangent);

        if(sliding_len <= 0.f) {
            return result;
        }
        auto tangent = delta_p_tangent / sliding_len;
        if(sliding_len < sfriction * penetration){
            delta_lagrange = applyPositionalCorrection(PositionalCorrectionInfo(tangent, e1, r1, &rb1, e2, r2, &rb2), sliding_len, tangent, delT);
        }
        return result;
    }
    std::vector<CollidingPair> PhysicsSystem::m_broadPhase() {
        return SweepBroadPhase().findPotentialPairs(entities.begin(), entities.end());
    }
    std::vector<PhysicsSystem::PenetrationConstraint> PhysicsSystem::m_narrowPhase(ColliderSystem& col_sys,const std::vector<CollidingPair>& pairs, float delT) {
        std::vector<PenetrationConstraint> result;
        for(const auto [e1, e2, s1i, s2i] : pairs) {
            auto res = m_handleCollision(e1, s1i, e2, s2i, delT);
            if(res.detected) {
                col_sys.updateInstant(e1);
                col_sys.updateInstant(e2);
                result.push_back(res);
            }
        }
        return result;
    }
    //need to update colliders after
    void PhysicsSystem::m_solveVelocities(std::vector<PenetrationConstraint>& constraints, float delT) {
        for(const auto& constraint : constraints) {
            const auto e1 = constraint.entity1;
            const auto e2 = constraint.entity2;

            const auto& trans1 = getComponent<Transform>(e1);
            auto& rb1 = getComponent<Rigidbody>(e1);

            const auto& trans2 = getComponent<Transform>(e2);
            auto& rb2 = getComponent<Rigidbody>(e2);

            const auto restitution = constraint.restitution;

            const auto pre_r1model = rotateVec(constraint.radius1, constraint.rot1_pre_col);
            const auto pre_r2model = rotateVec(constraint.radius2, constraint.rot2_pre_col);
            const auto pre_solve_contact_vel1 = m_calcContactVel(rb1.vel_pre_solve, rb1.ang_vel_pre_solve, pre_r1model);
            const auto pre_solve_contact_vel2 = m_calcContactVel(rb2.vel_pre_solve, rb2.ang_vel_pre_solve, pre_r2model);
            const auto pre_solve_relative_vel = pre_solve_contact_vel1 - pre_solve_contact_vel2;
            const auto pre_solve_normal_speed = dot(pre_solve_relative_vel, constraint.normal);

            const auto r1model = rotateVec(constraint.radius1, trans1.rotation);
            const auto r2model = rotateVec(constraint.radius2, trans2.rotation);
            const auto contact_vel1 = m_calcContactVel(rb1.vel, rb1.ang_vel, r1model);
            const auto contact_vel2 = m_calcContactVel(rb2.vel, rb2.ang_vel, r2model);
            const auto relative_vel = contact_vel1 - contact_vel2;
            const auto normal_speed = dot(relative_vel, constraint.normal);

            const auto tangent_vel = relative_vel - constraint.normal * normal_speed;
            const auto tangent_speed = length(tangent_vel);

            vec2f p = {0, 0};
            auto restitution_speed = m_calcRestitution(restitution, normal_speed, pre_solve_normal_speed, {0.f, gravity}, delT);
            if(abs(restitution_speed) > 0.f){
                const auto w1 = rb1.generalizedInverseMass(r1model, constraint.normal);
                const auto w2 = rb2.generalizedInverseMass(r2model, constraint.normal);
                const auto restitution_impulse = restitution_speed / (w1 + w2);
                p += restitution_impulse * constraint.normal;
            }

            const float sfriction = constraint.sfriction;
            const float dfriction = constraint.dfriction;
            // Compute dynamic friction
            if(abs(tangent_speed) > 0.f){
                const auto tangent = tangent_vel / tangent_speed;
                const auto w1 = rb1.generalizedInverseMass(r1model, tangent);
                const auto w2 = rb2.generalizedInverseMass(r2model, tangent);
                const auto friction_impulse =
                    m_calcDynamicFriction(dfriction, tangent_speed, w1 + w2, constraint.normal_lagrange, delT);
                p += friction_impulse * tangent;
                // constraint.contact.tangent_impulse += friction_impulse;
            }
            if(!rb1.isStatic) {
                const auto delta_lin_vel = p / rb1.mass();
                const auto delta_ang_vel = cross(r1model, p) / rb1.inertia();
                rb1.vel += delta_lin_vel;
                rb1.ang_vel += delta_ang_vel;
            }
            if(!rb2.isStatic) {
                const auto delta_lin_vel = p / rb2.mass();
                const auto delta_ang_vel = cross(r2model, p) / rb2.inertia();
                rb2.vel -= delta_lin_vel;
                rb2.ang_vel -= delta_ang_vel;
            }
        }
    }
    void PhysicsSystem::m_step(TransformSystem& trans_sys, ColliderSystem& col_sys, RigidbodySystem& rb_sys,ConstraintSystem& const_sys, float deltaTime) {
        rb_sys.integrate(deltaTime);
        trans_sys.update();
        col_sys.update();
        const_sys.update(deltaTime);
        auto potential_pairs = m_broadPhase();
        auto penetrations = m_narrowPhase(col_sys, potential_pairs, deltaTime);;
        trans_sys.update();
        rb_sys.deriveVelocities(deltaTime);
        m_solveVelocities(penetrations, deltaTime);
    }
    void PhysicsSystem::update(TransformSystem& trans_sys, ColliderSystem& col_sys, RigidbodySystem& rb_sys,ConstraintSystem& const_sys, float delT) {
        EMP_DEBUGCALL(debug_contactpoints.clear();)
        for(const auto e : entities){
            auto& rb = getComponent<Rigidbody>(e);
            if(!rb.isStatic) {
                rb.force += vec2f(0, gravity) * (float)substep_count * rb.mass();
            }
        }
        for(int i = 0; i < substep_count; i++) {
            m_step(trans_sys, col_sys, rb_sys, const_sys, delT / (float)substep_count);
            for(const auto e : entities) {
                auto& rb = getComponent<Rigidbody>(e);
                rb.force = {0, 0};
                rb.torque = 0.f;
            }
        }
    }
    void PhysicsSystem::onEntityAdded(Entity entity) {
        auto& rb = getComponent<Rigidbody>(entity);
        if(!rb.useAutomaticMass)
            return;
        auto& col = getComponent<Collider>(entity);
        rb.real_mass = col.area * rb.real_density;
        rb.real_inertia = col.inertia_dev_mass * rb.real_density;
    }
};
