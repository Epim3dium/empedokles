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

struct ColorComponent {
    sf::Color fill_color;
    sf::Color outline_color;
};
class DrawingSystem : public SystemOf<Collider, ColorComponent> {
public:
    void draw(sf::RenderTarget& target) {
        sf::CircleShape cs(5.f);
        cs.setOrigin(5.f, 5.f);
        cs.setFillColor(sf::Color::Red);
        for(const auto e : entities) {
            auto& collider = coordinator.getComponent<Collider>(e);
            auto& color = coordinator.getComponent<ColorComponent>(e);
            for(auto poly : collider.transformed_shape) {
                auto prev = poly.back();
                sf::Vertex verts[3];
                verts[2].position = poly.front();
                verts[0].color = color.fill_color;
                verts[1].color = color.fill_color;
                verts[2].color = color.fill_color;
                for(auto p : poly) {
                    verts[0].position = prev;
                    verts[1].position = p;
                    prev = p;
                    target.draw(verts, 3U, sf::Triangles);
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
        Rigidbody rb; rb.prev_pos = trans.position; rb.useAutomaticMass = true;
        Material mat;
        
        float t = ((double) rand() / (RAND_MAX)) + 1;
        float tt = ((double) rand() / (RAND_MAX)) + 1;
        float ttt = ((double) rand() / (RAND_MAX)) + 1;
        const unsigned char min_grey = 50;
        const unsigned char max_grey = 200;
        const unsigned char diff = max_grey - min_grey;
        ColorComponent color; color.fill_color = sf::Color(min_grey + diff * t, min_grey + diff * tt, min_grey + diff * ttt);

        coordinator.addComponent(e, color); 
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

        coordinator.registerComponent<ColorComponent>();


        transform_sys = coordinator.registerSystem<TransformSystem>();
        rigidbody_sys = coordinator.registerSystem<RigidbodySystem>();
        collider_sys  = coordinator.registerSystem<ColliderSystem>();
        physics_sys   = coordinator.registerSystem<PhysicsSystem>();

        drawing_sys   = coordinator.registerSystem<DrawingSystem>();

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

