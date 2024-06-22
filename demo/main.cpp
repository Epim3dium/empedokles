#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Window/Event.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "core/app.hpp"
#include "math/math.hpp"
#include "core/transform.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
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

class Demo : public App {
    public:
    std::vector<Body*> bodies;
    PhysicsSystem phy_sys;
    std::vector<vec2f> points;
    vec2f center = window_size  / 2.f;
    static constexpr float radius = 200;
    static constexpr float gravity = 500.f * 8.f;

    Body* createBody(std::vector<vec2f> point_cloud, float density = 1.f) {
        if(!isTriangulable(point_cloud)) {
            std::reverse(point_cloud.begin(), point_cloud.end());
        }
        auto b = new Body(point_cloud, density);
        phy_sys.add(&b->transform, &b->collider, &b->rigidbody, &b->material);
        return b;
    }
    bool setup(sf::Window& window) override {
        Body* platform = createBody({window_size, vec2f(10.f, window_size.y), vec2f(10.f, window_size.y - 50.f),
                                   vec2f(window_size.x, window_size.y - 50.f)}, INFINITY);
        platform->rigidbody.isStatic = true;
        bodies.push_back(platform);

        return true;
    }

    void update(sf::Time dt) override {
        if(keys[sf::Keyboard::P].pressed) {
            points.push_back(mouse_position);
            EMP_LOG_DEBUG << "added point";
        }
        if(keys[sf::Keyboard::R].pressed) {
            points.clear();
            EMP_LOG_DEBUG << "points reset";
        }
        if(keys[sf::Keyboard::Enter].pressed) {
            auto body = createBody(points);
            bodies.push_back(body);
            points.clear();
        }
        if(keys[sf::Keyboard::Tab].pressed) {
            auto body = createBody(points, INFINITY);
            body->rigidbody.isStatic = true;
            bodies.push_back(body);
            points.clear();
        }
        for(auto b : bodies) {
            b->transform.update();
            b->collider.update();
            b->rigidbody.update();
        }
        phy_sys.substeps(dt.asSeconds(), gravity);

    }
    void render(sf::RenderWindow& window) override {
        const float point_radius = 5.f;
        sf::CircleShape cs(point_radius);
        cs.setFillColor(sf::Color::Red);
        cs.setOrigin(point_radius, point_radius);
        for(auto p : points ) {
            cs.setPosition(p);
            window.draw(cs);
        }
        {
            cs.setFillColor(sf::Color::Magenta);
            for(auto p : phy_sys.contactpoints ) {
                cs.setPosition(p);
                window.draw(cs);
            }
            phy_sys.contactpoints.clear();
        }

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

