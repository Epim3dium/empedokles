#include "physics_system.hpp"
namespace emp {
    float PhysicsSystem::m_calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT) {
        // TODO: The XPBD paper has this, but it seems to be prevent bounces in cases
        // where bodies should clearly be bouncing.
        // Maybe change the threshold to be even lower? Or is this even needed at all?
        /*
        // If normal velocity is small enough, use restitution of 0 to avoid jittering
        if normal_speed.abs() <= 2.0 * gravity.length() * sub_dt {
            coefficient = 0.0;
        }
        */
        return fmin(-normal_speed + (-coef * pre_solve_norm_speed), 0.f);
    }
    float PhysicsSystem::m_calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange, float sub_dt) {
        auto normal_impulse = normal_lagrange / sub_dt;
        return fmin(-(coef * abs(normal_impulse)), (tangent_speed / generalized_inv_mass_sum));
    }
    float PhysicsSystem::m_applyPositionalCorrection(PhysicsManagerEntry& b1, PhysicsManagerEntry& b2, float c, vec2f normal, vec2f radius1, vec2f radius2, float delT, float compliance) {
        const vec2f& pos1 = b1.transform->position;
        const vec2f& pos2 = b2.transform->position;
        const float& rot1 = b1.transform->rotation;
        const float& rot2 = b2.transform->rotation;
        const float mass1 = b1.rigidbody->mass();
        const float mass2 = b2.rigidbody->mass();
        const float inertia1 = b1.rigidbody->inertia();
        const float inertia2 = b2.rigidbody->inertia();

        const auto r1 = rotateVec(radius1, b1.transform->rotation);
        const auto r2 = rotateVec(radius2, b2.transform->rotation);
        const auto w1 = b1.rigidbody->generalizedInverseMass(r1, normal);
        const auto w2 = b2.rigidbody->generalizedInverseMass(r2, normal);

        const auto tilde_compliance = compliance / (delT * delT);

        auto delta_lagrange = -c;
        delta_lagrange /= (w1 + w2 + tilde_compliance);

        auto p = delta_lagrange * normal;

        if(!b1.rigidbody->isStatic) {
            b1.transform->setPositionNow(pos1 + p / mass1);
            b1.transform->setRotationNow(rot1 + cross(r1, p) / inertia1);
            b1.collider->update();
        }

        if(!b2.rigidbody->isStatic) {
            b2.transform->setPositionNow(pos2 - p / mass2);
            b2.transform->setRotationNow(rot2 - cross(r2, p) / inertia2);
            b2.collider->update();
        }
        return delta_lagrange;
    }
    vec2f PhysicsSystem::m_calcContactVel(vec2f vel, float ang_vel, vec2f r) {
        return vel + ang_vel * vec2f(-r.y, r.x);
    }
    PhysicsSystem::PenetrationConstraint PhysicsSystem::m_handleCollision(PhysicsManagerEntry b1, const int convexIdx1, PhysicsManagerEntry b2, const int convexIdx2, float delT, float compliance) {
        PenetrationConstraint result;
        result.body1 = b1;
        result.body2 = b2;

        auto& rb1 = *b1.rigidbody;
        auto& rb2 = *b2.rigidbody;
        auto& col1 = *b1.collider;
        auto& col2 = *b2.collider;
        auto& trans1 = *b1.transform;
        auto& trans2 = *b2.transform;
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
        const float sfriction = 0.5f * (b1.material->static_friction + b2.material->static_friction);
        const float dfriction = 0.5f * (b1.material->dynamic_friction + b2.material->dynamic_friction);

        auto& intersectingShape1 = col1.constituentConvex()[convexIdx1];
        auto& intersectingShape2 = col2.constituentConvex()[convexIdx2];
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
        result.pos1_at_col = pos1;
        result.pos2_at_col = pos2;
        result.rot1_at_col = rot1;
        result.rot2_at_col = rot2;

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

        auto delta_lagrange = m_applyPositionalCorrection(b1, b2, penetration, normal, r1, r2, delT);
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
            delta_lagrange = m_applyPositionalCorrection(b1, b2, sliding_len, tangent, r1, r2, delT);
        }
        return result;
    }
    std::vector<BroadPhaseSolver::CollidingPair> PhysicsSystem::m_broadPhase(std::vector<PhysicsManagerEntry>& bodies) {
        return m_broad_phase_solver->findPotentialPairs(bodies);
    }
    std::vector<PhysicsSystem::PenetrationConstraint> PhysicsSystem::m_narrowPhase(const std::vector<BroadPhaseSolver::CollidingPair>& pairs, std::vector<PhysicsManagerEntry>& bodies, float delT) {
        std::vector<PenetrationConstraint> result;
        for(auto [b1i, b2i, s1i, s2i] : pairs) {
            auto b1 = bodies[b1i];
            auto b2 = bodies[b2i];
            auto& shape1 = b1.collider->constituentConvex();
            auto& shape2 = b2.collider->constituentConvex();
            auto res = m_handleCollision(b1, s1i, b2, s2i, delT);
            if(res.detected) {
                result.push_back(res);
            }
        }
        return result;
    }
    //need to update colliders after
    void PhysicsSystem::m_integrate(std::vector<PhysicsManagerEntry>& bodies, float h) {
        for(auto b : bodies) {
            b.rigidbody->integrate(h);
        }
    }
    //need to update colliders after
    void PhysicsSystem::m_deriveVelocities(std::vector<PhysicsManagerEntry>& bodies, float h) {
        for(auto b : bodies) {
            b.rigidbody->deriveVelocities(h);
        }
    }
    void PhysicsSystem::m_solveVelocities(std::vector<PenetrationConstraint>& constraints, float delT) {
        for(auto& constraint : constraints) {
            auto& b1 = constraint.body1;
            auto& b2 = constraint.body2;
            auto& rb1 = *b1.rigidbody;
            auto& rb2 = *b2.rigidbody;
            auto& trans1 = *b1.transform;
            auto& trans2 = *b2.transform;

            const auto pre_r1model = rotateVec(constraint.radius1, constraint.rot1_at_col);
            const auto pre_r2model = rotateVec(constraint.radius2, constraint.rot2_at_col);
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
            constexpr float rest = 0.1f;
            auto restitution_speed = m_calcRestitution(rest, normal_speed, pre_solve_normal_speed, {}, delT);
            if(abs(restitution_speed) > 0.f){
                const auto w1 = rb1.generalizedInverseMass(r1model, constraint.normal);
                const auto w2 = rb2.generalizedInverseMass(r2model, constraint.normal);
                const auto restitution_impulse = restitution_speed / (w1 + w2);
                p += restitution_impulse * constraint.normal;
            }

            constexpr float dfric = 0.4f;
            // Compute dynamic friction
            if(abs(tangent_speed) > 0.f){
                const auto tangent = tangent_vel / tangent_speed;
                const auto w1 = rb1.generalizedInverseMass(r1model, tangent);
                const auto w2 = rb2.generalizedInverseMass(r2model, tangent);
                const auto friction_impulse =
                    m_calcDynamicFriction(dfric, tangent_speed, w1 + w2, constraint.normal_lagrange, delT);
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
    void PhysicsSystem::m_step(std::vector<PhysicsManagerEntry> bodies, float deltaTime) {
        m_integrate(bodies, deltaTime);
        for(auto b : bodies) {
            b.transform->update();
            b.collider->update();
        }
        auto potential_pairs = m_broadPhase(bodies);
        auto penetrations = m_narrowPhase(potential_pairs, bodies, deltaTime);;
        m_deriveVelocities(bodies, deltaTime);
        m_solveVelocities(penetrations, deltaTime);
    }
    void PhysicsSystem::substeps(float delT, float gravity, size_t substepCount) {
        EMP_DEBUGCALL(debug_contactpoints.clear();)
        for(int i = 0; i < substepCount; i++) {
            for(auto b : m_entries ){
                if(!b.rigidbody->isStatic) {
                    b.rigidbody->force += vec2f(0, gravity / (float)substepCount) * b.rigidbody->mass();
                }
            }
            m_step(m_entries, delT / (float)substepCount);
            for(auto b : m_entries) {
                b.rigidbody->force = {0, 0};
                b.rigidbody->torque = 0.f;
            }
        }
    }
};
