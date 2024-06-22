#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "core/app.hpp"
#include "math/math.hpp"
#include "core/transform.hpp"
#include "physics/material.hpp"
#include "physics/rigidbody.hpp"
using namespace emp;

struct Point {
    vec2f pos;
    vec2f last_pos;
    vec2f force;
    float mass;
    static constexpr float radius = 5.f;
    Point(vec2f p, float m = 1.f) : pos(p), last_pos(p), force(0, 0), mass(m) {}
};
void integrate(Point& p, float deltaTime) {
    vec2f acc = p.force / p.mass;
    vec2f prev = p.pos;
    p.pos = 2.f * p.pos - p.last_pos + acc * (deltaTime * deltaTime);
    p.last_pos = prev;
    p.force = vec2f(0, 0);
}
void resolveCollision(Point& p1, Point& p2, float deltaTime) {
    auto ql = qlen(p1.pos - p2.pos);
    auto rad_sum = p1.radius + p2.radius;
    if(ql < rad_sum * rad_sum) {
        auto l = sqrt(ql);

        auto mag = l - rad_sum;
        mag /= (p1.mass + p2.mass);
        auto dir = (p1.pos - p2.pos) / l;
        p1.pos -= dir * mag * p2.mass * 0.5f;
        p2.pos += dir * mag * p1.mass * 0.5f;
    }
}
struct Body {
    Transform transform;
    Collider collider;
    Rigidbody rigidbody;
    Material material;
    Body(std::vector<vec2f> point_cloud, float density = 1.f)  :
        transform(calculateMassInertiaArea(point_cloud).centroid),
        collider(point_cloud, &transform, true),
        rigidbody(&transform, &collider, density),
        material()
    {
        EMP_LOG_DEBUG << "created body; mass: " << rigidbody.mass() << "\tinertia: " << rigidbody.inertia()
            << "\tat: " << transform.position.x << ", " << transform.position.y;
    }
};
struct PenetrationConstraint {
    bool detected = false;
    Body* body1;
    Body* body2;
    vec2f normal;
    float penetration;
    vec2f pos1_at_col;
    vec2f pos2_at_col;
    float rot1_at_col;
    float rot2_at_col;
    //not rotated not translated
    vec2f radius1;
    //not rotated not translated
    vec2f radius2;
    float normal_lagrange;
};
float calcRestitution(float coef, float normal_speed, float pre_solve_norm_speed, vec2f gravity, float delT) {
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
float calcDynamicFriction(float coef, float tangent_speed, float generalized_inv_mass_sum, float normal_lagrange, float sub_dt) {
    auto normal_impulse = normal_lagrange / sub_dt;
    return fmin(-(coef * abs(normal_impulse)), (tangent_speed / generalized_inv_mass_sum));
}
float applyPositionalCorrection(Body& b1, Body& b2, float c, vec2f normal, vec2f radius1, vec2f radius2, float delT, float compliance = 0.f) {
    const vec2f& pos1 = b1.transform.position;
    const vec2f& pos2 = b2.transform.position;
    const float& rot1 = b1.transform.rotation;
    const float& rot2 = b2.transform.rotation;
    const float mass1 = b1.rigidbody.mass();
    const float mass2 = b2.rigidbody.mass();
    const float inertia1 = b1.rigidbody.inertia();
    const float inertia2 = b2.rigidbody.inertia();

    const auto r1 = rotateVec(radius1, b1.transform.rotation);
    const auto r2 = rotateVec(radius2, b2.transform.rotation);
    const auto w1 = b1.rigidbody.generalizedInverseMass(r1, normal);
    const auto w2 = b2.rigidbody.generalizedInverseMass(r2, normal);

    const auto tilde_compliance = compliance / (delT * delT);

    auto delta_lagrange = -c;
    delta_lagrange /= (w1 + w2 + tilde_compliance);

    auto p = delta_lagrange * normal;

    b1.transform.setPositionNow(pos1 + p / mass1);
    b2.transform.setPositionNow(pos2 - p / mass2);

    b1.transform.setRotationNow(rot1 + cross(r1, p) / inertia1);
    b2.transform.setRotationNow(rot2 - cross(r2, p) / inertia2);
    b1.collider.update();
    b2.collider.update();
    return delta_lagrange;
}
std::vector<vec2f> contactpoints;
vec2f calcContactVel(vec2f vel, float ang_vel, vec2f r) {
    return vel + ang_vel * vec2f(-r.y, r.x);
}
PenetrationConstraint handleCollision(Body& b1, const int convexIdx1, Body& b2, const int convexIdx2, float delT, float compliance = 0.f) {
    PenetrationConstraint result;
    result.body1 = &b1;
    result.body2 = &b2;

    const vec2f& pos1 = b1.transform.position;
    const vec2f& pos2 = b2.transform.position;
    const float& rot1 = b1.transform.rotation;
    const float& rot2 = b2.transform.rotation;
    const float mass1 = b1.rigidbody.mass();
    const float mass2 = b2.rigidbody.mass();
    const float inertia1 = b1.rigidbody.inertia();
    const float inertia2 = b2.rigidbody.inertia();
    
    if(b1.rigidbody.isStatic && b2.rigidbody.isStatic)
        return result;
    const float sfriction = 0.5f * (b1.material.static_friction + b2.material.static_friction);
    const float dfriction = 0.5f * (b1.material.dynamic_friction + b2.material.dynamic_friction);

    auto& intersectingShape1 = b1.collider.constituentConvex()[convexIdx1];
    auto& intersectingShape2 = b2.collider.constituentConvex()[convexIdx2];
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
    if(d > penetration * 2.f) {
        EMP_LOG_DEBUG << "contact points bug";
        return {false};
    }
    contactpoints.push_back(intersection.cp1);
    contactpoints.push_back(intersection.cp2);

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

    auto delta_lagrange = applyPositionalCorrection(b1, b2, penetration, normal, r1, r2, delT);
    result.normal_lagrange = delta_lagrange;
    const auto normal_impulse = delta_lagrange / delT;

    auto delta_p1 = pos1 - b1.rigidbody.prev_pos
        + rotateVec(r1, rot1)
        - rotateVec(r1, b1.rigidbody.prev_rot);
    auto delta_p2 = pos2 - b2.rigidbody.prev_pos
        + rotateVec(r2, rot2)
        - rotateVec(r2, b2.rigidbody.prev_rot);
    auto delta_p = delta_p1 - delta_p2;
    auto delta_p_tangent = delta_p - dot(delta_p, normal) * normal;
    auto sliding_len = length(delta_p_tangent);

    if(sliding_len <= 0.f) {
        return result;
    }
    auto tangent = delta_p_tangent / sliding_len;
    if(sliding_len < sfriction * penetration){
        delta_lagrange = applyPositionalCorrection(b1, b2, sliding_len, tangent, r1, r2, delT);
    }
    return result;
}
std::vector<PenetrationConstraint> detectPenetrations(std::vector<Body*>& bodies, float delT) {
    std::vector<PenetrationConstraint> result;
    for(int i = 0; i < bodies.size(); i++) {
        for(int ii = i + 1; ii < bodies.size(); ii++) {
            auto& b1 = *bodies[i];
            auto& b2 = *bodies[ii];
            auto& shape1 = bodies[i]->collider.constituentConvex();
            auto& shape2 = bodies[ii]->collider.constituentConvex();
            for(int j = 0; j < shape1.size(); j++) {
                for(int k = 0; k < shape2.size(); k++) {
                    auto res = handleCollision(*bodies[i], j, *bodies[ii], k, delT);
                    if(res.detected) {
                        result.push_back(res);
                    }
                }
            }
        }
    }
    return result;
}
//need to update colliders after
void integrate(std::vector<Body*>& bodies, float h) {
    for(auto b : bodies) {
        auto& curr = *b;
        b->rigidbody.integrate(h);
    }
}
//need to update colliders after
void deriveVelocities(std::vector<Body*>& bodies, float h) {
    for(auto b : bodies) {
        auto& curr = *b;
        b->rigidbody.deriveVelocities(h);
    }
}
void solveVelocities(std::vector<PenetrationConstraint>& constraints, float delT) {
    for(auto& constraint : constraints) {
        auto& b1 = *constraint.body1;
        auto& b2 = *constraint.body2;
        auto& rb1 = b1.rigidbody;
        auto& rb2 = b2.rigidbody;

        const auto pre_r1model = rotateVec(constraint.radius1, constraint.rot1_at_col);
        const auto pre_r2model = rotateVec(constraint.radius2, constraint.rot2_at_col);
        const auto pre_solve_contact_vel1 = calcContactVel(rb1.vel_pre_solve, rb1.ang_vel_pre_solve, pre_r1model);
        const auto pre_solve_contact_vel2 = calcContactVel(rb2.vel_pre_solve, rb2.ang_vel_pre_solve, pre_r2model);
        const auto pre_solve_relative_vel = pre_solve_contact_vel1 - pre_solve_contact_vel2;
        const auto pre_solve_normal_speed = dot(pre_solve_relative_vel, constraint.normal);

        const auto r1model = rotateVec(constraint.radius1, b1.transform.rotation);
        const auto r2model = rotateVec(constraint.radius2, b2.transform.rotation);
        const auto contact_vel1 = calcContactVel(rb1.vel, rb1.ang_vel, r1model);
        const auto contact_vel2 = calcContactVel(rb2.vel, rb2.ang_vel, r2model);
        const auto relative_vel = contact_vel1 - contact_vel2;
        const auto normal_speed = dot(relative_vel, constraint.normal);

        const auto tangent_vel = relative_vel - constraint.normal * normal_speed;
        const auto tangent_speed = length(tangent_vel);

        vec2f p = {0, 0};
        constexpr float rest = 0.1f;
        auto restitution_speed = calcRestitution(rest, normal_speed, pre_solve_normal_speed, {}, delT);
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
                calcDynamicFriction(dfric, tangent_speed, w1 + w2, constraint.normal_lagrange, delT);
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
void step(std::vector<Body*> bodies, float deltaTime) {
    integrate(bodies, deltaTime);
    for(auto b : bodies) {
        b->transform.update();
        b->collider.update();
    }
    auto penetrations = detectPenetrations(bodies, deltaTime);;
    deriveVelocities(bodies, deltaTime);
    solveVelocities(penetrations, deltaTime);
}
void substeps(std::vector<Body*> bodies, float delT, float gravity, size_t substepCount = 8U) {
    for(int i = 0; i < substepCount; i++) {
        for(auto b : bodies ){
            if(!b->rigidbody.isStatic) {
                b->rigidbody.force += vec2f(0, gravity / (float)substepCount) * b->rigidbody.mass();
            }
        }
        step(bodies, delT / (float)substepCount);
        for(auto b : bodies) {
            auto& curr = *b;
            curr.rigidbody.force = {0, 0};
            curr.rigidbody.torque = 0.f;
        }
    }
}

class Demo : public App {
    public:
    std::vector<Body*> bodies;
    std::vector<vec2f> points;
    vec2f center = window_size  / 2.f;
    static constexpr float radius = 200;
    static constexpr float gravity = 500.f * 8.f;

    Body* createBody(std::vector<vec2f> point_cloud, float density = 1.f) {
        if(!isTriangulable(point_cloud)) {
            std::reverse(point_cloud.begin(), point_cloud.end());
        }
        auto b = new Body(point_cloud, density);
        return b;
    }
    bool setup(sf::Window& window) override {
        Body* platform = createBody({window_size, vec2f(10.f, window_size.y), vec2f(10.f, window_size.y - 50.f),
                                   vec2f(window_size.x, window_size.y - 50.f)}, 10000.f);
        platform->rigidbody.isStatic = true;
        bodies.push_back(platform);

        return true;
    }

    void update(sf::Time dt) override {
        if(keys[sf::Keyboard::P].pressed) {
            points.push_back(mouse_position);
            EMP_LOG_DEBUG << "added point";
        }
        if(keys[sf::Keyboard::Enter].pressed) {
            auto body = createBody(points);
            bodies.push_back(body);
            points.clear();
        }
        if(keys[sf::Keyboard::Tab].pressed) {
            auto body = createBody(points, 10000.f);
            body->rigidbody.isStatic = true;
            bodies.push_back(body);
            points.clear();
        }
        for(auto b : bodies) {
            b->transform.update();
            b->collider.update();
            b->rigidbody.update();
        }
        substeps(bodies, dt.asSeconds(), gravity);

    }
    void render(sf::RenderWindow& window) override {
        sf::CircleShape cs(radius + Point::radius);

        cs.setOrigin(radius + Point::radius, radius + Point::radius);
        cs.setPosition(window_size / 2.f);
        cs.setFillColor(sf::Color::Blue);
        window.draw(cs);

        cs.setRadius(Point::radius);
        cs.setFillColor(sf::Color::White);
        cs.setOrigin(5.f, 5.f);
        for(auto p : points ) {
            cs.setPosition(p);
            window.draw(cs);
        }
        for(auto p : contactpoints ) {
            cs.setPosition(p);
            window.draw(cs);
        }
        static size_t last_c_p_s = 0;
        if(last_c_p_s != 0) 
            while(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) ;;
        last_c_p_s = contactpoints.size();
        contactpoints.clear();
        cs.setFillColor(sf::Color::Red);
        sf::Color color = sf::Color::Green;

        for(auto bptr : bodies) {
            auto& b_many = bptr->collider.constituentConvex();
            for(auto b : b_many) {
                auto prev = b.back();
                for(auto p : b) {
                    sf::Vertex verts[2];
                    verts[0].position = prev;
                    verts[1].position = p;
                    if(bptr->rigidbody.isStatic) {
                        verts[0].color = sf::Color::Yellow;
                        verts[1].color = sf::Color::Yellow;
                    }
                    prev = p;
                    window.draw(verts, 2U, sf::Lines);
                }
            }
        }
    }
};

int main()
{
    Demo demo;
    run(demo);


    return 0;
}

