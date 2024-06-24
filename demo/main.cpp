#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "core/app.hpp"
#include "core/transform.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
#include "physics/rigidbody.hpp"
using namespace emp;

struct Body {
    TransformInstance transform;
    ColliderInstance collider;
    RigidbodyInstance rigidbody;
    MaterialInstance material;
    Body(std::vector<vec2f> point_cloud, float density = 1.f)  :
        transform(calculateMassInertiaArea(point_cloud).centroid),
        collider(point_cloud, &transform, true),
        rigidbody(&transform, collider.ptr(), density),
        material()
    {
        EMP_LOG_DEBUG << "created body; mass: " << rigidbody.get().mass() << "\tinertia: " << rigidbody.get().inertia()
            << "\tat: " << transform.get().position.x << ", " << transform.get().position.y;
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
        phy_sys.add(b->transform.ptr(), b->collider.ptr(), b->rigidbody.ptr(), b->material.ptr());
        return b;
    }
    bool setup(sf::Window& window) override {
        ColliderSystem::allocateMemory();
        RigidbodySystem::allocateMemory();
        TransformSystem::allocateMemory();
        MaterialSystem::allocateMemory();

        Body* platform = createBody({window_size, vec2f(10.f, window_size.y), vec2f(10.f, window_size.y - 50.f),
                                   vec2f(window_size.x, window_size.y - 50.f)}, INFINITY);
        platform->rigidbody.get().isStatic = true;
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
            body->rigidbody.get().isStatic = true;
            bodies.push_back(body);
            points.clear();
        }
        for(auto b : bodies) {
            b->transform.get().update();
            b->collider.get().update();
            b->rigidbody.get().update();
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
        EMP_DEBUGCALL(
            cs.setFillColor(sf::Color::Transparent);
            cs.setOutlineColor(sf::Color::Green);
            cs.setOutlineThickness(1.f);
            for(auto p : phy_sys.debug_contactpoints) {
                cs.setPosition(p);
                window.draw(cs);
            }
        );

        sf::Color color = sf::Color::Green;

        for(auto bptr : bodies) {
            auto& b_many = bptr->collider.get().constituentConvex();
            for(auto b : b_many) {
                auto prev = b.back();
                for(auto p : b) {
                    sf::Vertex verts[2];
                    verts[0].position = prev;
                    verts[1].position = p;
                    if(bptr->rigidbody.get().isStatic) {
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
    {
        Demo demo;
        run(demo);
    }
    return 0;
}

