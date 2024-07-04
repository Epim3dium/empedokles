#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "math/geometry_func.hpp"
#include "scene/app.hpp"
#include "scene/transform.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
#include "physics/rigidbody.hpp"
using namespace emp;

class DrawingSystem : public SystemOf<Collider> {
public:
    void draw(sf::RenderTarget& target) {
        for(const auto e : entities) {
            auto& col = coordinator.getComponent<Collider>(e);
            for(auto poly : col.transformed_shape) {
                auto prev = poly.back();
                for(auto p : poly) {
                    sf::Vertex verts[2];
                    verts[0].position = prev;
                    verts[1].position = p;
                    prev = p;
                    target.draw(verts, 2U, sf::Lines);
                }
            }
        }
    }
};

class Demo : public App {
    public:
    std::shared_ptr<DrawingSystem> drawing_sys;

    std::shared_ptr<PhysicsSystem> physics_sys;
    std::shared_ptr<RigidbodySystem> rigidbody_sys;
    std::shared_ptr<ColliderSystem> collider_sys;
    std::shared_ptr<TransformSystem> transform_sys;
    std::vector<vec2f> points;
    vec2f center = window_size  / 2.f;
    static constexpr float radius = 200;
    static constexpr float gravity = 500.f * 8.f;

    Entity createBody(std::vector<vec2f> point_cloud, float density = 1.f) {
        if(!isTriangulable(point_cloud)) {
            std::reverse(point_cloud.begin(), point_cloud.end());
        }
        auto e = coordinator.createEntity();
        EMP_LOG_DEBUG << "new entity: " << e;
        Transform trans(calculateMassInertiaArea(point_cloud).centroid);
        Collider col(point_cloud, true);
        Rigidbody rb; rb.prev_pos = trans.position;
        rb.useAutomaticMass = true;
        Material mat;

        coordinator.addComponent(e, trans); 
        coordinator.addComponent(e, col); 
        coordinator.addComponent(e, rb); 
        coordinator.addComponent(e, mat); 
        return e;
    }
    bool setup(sf::Window& window) override {
        coordinator.init();

        coordinator.registerComponent<Transform>();
        coordinator.registerComponent<Material>();
        coordinator.registerComponent<Collider>();
        coordinator.registerComponent<Rigidbody>();

        transform_sys = coordinator.registerSystem<TransformSystem, Transform>();
        drawing_sys   = coordinator.registerSystem<DrawingSystem, Collider>();
        rigidbody_sys = coordinator.registerSystem<RigidbodySystem, Transform, Rigidbody>();
        collider_sys  = coordinator.registerSystem<ColliderSystem,  Transform, Collider>();
        physics_sys   = coordinator.registerSystem<PhysicsSystem,   Transform, Collider, Rigidbody, Material>();
        Entity platform = createBody({window_size, vec2f(10.f, window_size.y), vec2f(10.f, window_size.y - 50.f),
                                   vec2f(window_size.x, window_size.y - 50.f)}, INFINITY);
        coordinator.getComponent<Rigidbody>(platform).isStatic = true;

        physics_sys->gravity = 500.f;

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
            EMP_LOG_DEBUG << "created body from points";
            auto body = createBody(points);
            points.clear();
        }
        if(keys[sf::Keyboard::Tab].pressed) {
            auto body = createBody(points, INFINITY);
            points.clear();
        }
        rigidbody_sys->updateMasses();
        transform_sys->update();
        collider_sys->update();
        physics_sys->update(*transform_sys, *collider_sys, *rigidbody_sys, dt.asSeconds());

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
        drawing_sys->draw(window);
        EMP_DEBUGCALL(
            cs.setFillColor(sf::Color::Transparent);
            cs.setOutlineColor(sf::Color::Green);
            cs.setOutlineThickness(1.f);
            for(auto p : physics_sys->debug_contactpoints) {
                cs.setPosition(p);
                window.draw(cs);
            }
        );
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

