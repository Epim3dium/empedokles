#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "core/app.hpp"
#include "math/math.hpp"
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
    ConvexPolygon shape;
    float MMOI = 1.f;
    float mass = 1.f;
    
    vec2f prev_pos;
    vec2f velocity;
    vec2f velocity_carry;
    vec2f force;

    float prev_rot = 0.f;
    float angular_vel = 0.f;
    float ang_vel_carry = 0.f;
    float torque = 0.f;

    bool isStatic = false;
    Body(std::vector<vec2f> point_cloud, float thickness = 1.f, float density = 1.f) 
        : shape(ConvexPolygon::CreateFromPoints(point_cloud))
    {
        auto mmoi_info = calculateMMOI(shape.getModelVertecies(), thickness, density);
        MMOI = mmoi_info.MMOI;
        mass = mmoi_info.mass;
        EMP_LOG_DEBUG << "created body; mass: " << mass << "\tMMOI: " << MMOI;
    }
};
float handleCollision(Body& b1, Body& b2, float h, float vn_prev) {
    if(b1.isStatic && b2.isStatic)
        return 0.f;
    constexpr float a = 0.0f;
    auto intersection = intersectPolygonPolygon(b1.shape, b2.shape);
    if(!intersection.detected) {
        return 0.f;
    }
    float vn = 0.f;
    auto n = -intersection.contact_normal;
    for(auto cp : intersection.cp) {
        auto c = intersection.overlap / intersection.cp.size();
        const auto r1 = cp - b1.shape.getPos();
        const auto r2 = cp - b2.shape.getPos();
        const auto r1modeled = rotateVec(r1, -b1.shape.getRot());
        const auto r2modeled = rotateVec(r2, -b2.shape.getRot());

        const auto w1 = 1.f / b1.mass + (cross(r1, n) / b1.MMOI * cross(r1, n));
        const auto w2 = 1.f / b2.mass + (cross(r2, n) / b2.MMOI * cross(r2, n));

        const auto ahat = a / (h * h) ;

        auto dlambda = -c /* - ahat */;
        dlambda /= (w1 + w2 + ahat);
        //static friction
        const auto r1hat = b1.prev_pos + rotateVec(r1modeled, b1.prev_rot);
        const auto r2hat = b2.prev_pos + rotateVec(r2modeled, b2.prev_rot);
        const auto dp = (cp - r1hat) - (cp - r2hat);
        const auto dptangent = dp - (dot(dp, n)) * n;
        const auto lambdadp = dptangent / (w1 + w2 + ahat);

        auto p = dlambda * n;

        b1.shape.setPos(b1.shape.getPos() + p * w1);
        b2.shape.setPos(b2.shape.getPos() - p * w2);

        b1.shape.setRot(b1.shape.getRot() + cross(r1, p) / b1.MMOI);
        b2.shape.setRot(b2.shape.getRot() - cross(r2, p) / b2.MMOI);
        //applying friction
        //dynamic
        const vec2f r1perp(-r1.y, r1.x);
        const vec2f r2perp(-r2.y, r2.x);

        const auto p1ang_vel_lin = r1perp * b1.angular_vel;
        const auto p2ang_vel_lin = r2perp * b2.angular_vel;

        const auto vel_sum1 = b1.velocity;
        const auto vel_sum2 = b2.velocity;

        //calcauto relative velocity
        const auto rel_vel = vel_sum1 - vel_sum2;
        vn = dot(n, rel_vel);
        const auto tangent = rel_vel - n * vn;

        constexpr float dfric = 0.1f;
        constexpr float e = 0.0f;
        const auto normal_force = dlambda / (h * h);
        const auto vtl = length(tangent);
        const auto inv_vtl = (vtl == 0.f ? 0.f : 1.f / vtl);
        const auto dv_fric = -tangent * inv_vtl * fmin(h * dfric * abs(normal_force), vtl); 

        const auto dv_rest = n * (-vn + fmin(-e * vn_prev, 0.f));
        EMP_LOG_DEBUG << dv_rest.x << " : " << dv_rest.y << "\t\t" << vn;

        p = dv_fric / (w1 + w2); 
        b1.velocity_carry += p * w1;
        b2.velocity_carry -= p * w2;

        // p = dv_rest / (w1 + w2); 
        // b1.velocity_carry += p * w1;
        // b2.velocity_carry -= p * w2;

        // b1.ang_vel_carry += 0.5f * cross(r1, p) / b1.MMOI;
        // b2.ang_vel_carry -= 0.5f * cross(r2, p) / b2.MMOI;
    }
    return vn;
}
void step(std::vector<Body*> bodies, float h) {
    for(auto b : bodies) {
        auto& curr = *b;
        curr.prev_pos = curr.shape.getPos();
        curr.velocity += h * curr.force / curr.mass;
        curr.shape.setPos(curr.shape.getPos() + curr.velocity * h);

        curr.prev_rot = curr.shape.getRot();
        curr.angular_vel += h * curr.torque / curr.MMOI;
        curr.shape.setRot(curr.shape.getRot() + curr.angular_vel * h);
    }
    struct LagrangeInfo {
        vec2f dir;
        float mag = 0.f;
    };
    static std::unordered_map<int, vec2f> vn_prev;
    //solvePositions
    for(int i = 0; i < bodies.size(); i++) {
        for(int ii = i + 1; ii < bodies.size(); ii++) {
            handleCollision(*bodies[i], *bodies[ii], h, 0.f);
        }
    }

    for(auto b : bodies) {
        auto& curr = *b;
        if(curr.isStatic)
            continue;
        curr.velocity = (curr.shape.getPos() - curr.prev_pos) / h + curr.velocity_carry;
        curr.velocity_carry = vec2f(0, 0);
        curr.angular_vel = (curr.shape.getRot() - curr.prev_rot) / h + curr.ang_vel_carry;
        curr.ang_vel_carry = 0;
    }
}
void substeps(std::vector<Body*> bodies, float delT, size_t substepCount = 8U) {
    for(int i = 0; i < substepCount; i++) {
        step(bodies, delT / (float)substepCount);
    }
    for(auto b : bodies) {
        auto& curr = *b;
        curr.force = {0, 0};
        curr.torque = 0.f;
    }
}

class Demo : public App {
    public:
    std::vector<Body*> bodies;
    std::vector<vec2f> points;
    vec2f center = window_size  / 2.f;
    static constexpr float radius = 200;
    static constexpr float gravity = 500.f;
    IntersectionPolygonPolygonResult info;

    bool setup(sf::Window& window) override {
        Body* platform = new Body({window_size, vec2f(10.f, window_size.y), vec2f(10.f, window_size.y - 50.f),
                                   vec2f(window_size.x, window_size.y - 50.f)},
                                  1000.f, 1000.f);
        platform->isStatic = true;
        bodies.push_back(platform);

        return true;
    }

    void update(sf::Time dt) override {
        if(keys[sf::Keyboard::P].pressed) {
            points.push_back(mouse_position);
            EMP_LOG_DEBUG << "added point";
        }
        if(keys[sf::Keyboard::Enter].pressed) {
            auto body = new Body(points);
            bodies.push_back(body);
            points.clear();
        }
        if(keys[sf::Keyboard::Tab].pressed) {
            auto body = new Body(points, 100.f, 100.f);
            body->isStatic = true;
            bodies.push_back(body);
            points.clear();
        }
        for(auto b : bodies ){
            if(!b->isStatic)
                b->force += vec2f(0, gravity) * b->mass;
        }
        substeps(bodies, dt.asSeconds());

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
        cs.setFillColor(sf::Color::Red);
        sf::Color color = sf::Color::Green;
        for(auto p : info.cp ) {
            sf::Vertex verts[2];
            verts[0].color = color;
            verts[1].color = color;
            verts[0].position = p + info.contact_normal * 10.f;
            verts[1].position = p;
            window.draw(verts, 2U, sf::Lines);
        }

        for(auto b : bodies) {
            auto prev = b->shape.getVertecies().back();
            for(auto p : b->shape.getVertecies()) {
                sf::Vertex verts[2];
                verts[0].position = prev;
                verts[1].position = p;
                if(b->isStatic) {
                    verts[0].color = sf::Color::Yellow;
                    verts[1].color = sf::Color::Yellow;
                }
                prev = p;
                window.draw(verts, 2U, sf::Lines);
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

