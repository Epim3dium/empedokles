#include "scene/app.hpp"
using namespace emp;
// class TempApp {
//     sf::Clock deltaClock;
// public:
//     struct KeyState {
//         bool held = false;
//         bool pressed = false;
//         bool released = false;
//     };
//     std::unordered_map<sf::Keyboard::Key, KeyState> keys;
//     std::unordered_map<sf::Mouse::Button, KeyState> buttons;
//     vec2f mouse_position;
//
//     vec2f window_size = {960, 540};
//     virtual bool setup(sf::Window& window) {
//         return true;
//     }
//     virtual void update(sf::Time deltaTime) {}
//     virtual void render(sf::RenderWindow& rw) {}
//     virtual void cleanup() {}
//
//     void resetKeys() {
//         for(auto& state : keys) {
//             state.second.pressed = false;
//             state.second.released = false;
//         }
//         for(auto& state : buttons) {
//             state.second.pressed = false;
//             state.second.released = false;
//         }
//     }
//     void handleEvent(const sf::Event& event) {
//         if(event.type == sf::Event::KeyPressed) {
//             if(!keys[event.key.code].held) {
//                 keys[event.key.code].pressed = true;
//             }else {
//                 keys[event.key.code].pressed = false;
//             }
//             keys[event.key.code].held = true;
//         }else if (event.type == sf::Event::KeyReleased) {
//             keys[event.key.code].held = false;
//             keys[event.key.code].pressed = false;
//             keys[event.key.code].released = true;
//         }else {
//             keys[event.key.code].released = false;
//             keys[event.key.code].pressed = false;
//         }
//
//         if(event.type == sf::Event::MouseButtonPressed) {
//             if(!buttons[event.mouseButton.button].held) {
//                 buttons[event.mouseButton.button].pressed = true;
//             }else {
//                 buttons[event.mouseButton.button].pressed = false;
//             }
//             keys[event.key.code].held = true;
//         }else if (event.type == sf::Event::MouseButtonReleased) {
//             buttons[event.mouseButton.button].held = false;
//             buttons[event.mouseButton.button].pressed = false;
//             buttons[event.mouseButton.button].released = true;
//         }else {
//             buttons[event.mouseButton.button].released = false;
//         }
//     }
//
//     
//     inline friend void run(TempApp& app) {
//         sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(app.window_size.x, app.window_size.y), "demo");
//         window.setFramerateLimit(60);
//         if(!app.setup(window)) {
//             return;
//         }
//         sf::Clock deltaClock;
//         while (window.isOpen())
//         {
//             app.mouse_position = (vec2f)sf::Mouse::getPosition(window);
//             sf::Time dt = deltaClock.restart();
//             sf::Event event;
//             vec2f mouse_pos = (vec2f)sf::Mouse::getPosition(window);
//
//             app.resetKeys();
//             while (window.pollEvent(event))
//             {
//                 if (event.type == sf::Event::Closed)
//                     window.close();
//                 app.handleEvent(event);
//             }
//             app.update(dt);
//             window.clear(sf::Color::Black);
//             app.render(window);
//             window.display();
//         }
//     }
// };
//
// struct ColorComponent {
//     sf::Color fill_color;
//     sf::Color outline_color;
// };
// class DrawingSystem : public SystemOf<Collider, ColorComponent> {
// public:
//     void draw(sf::RenderTarget& target) {
//         sf::CircleShape cs(5.f);
//         cs.setOrigin(5.f, 5.f);
//         cs.setFillColor(sf::Color::Red);
//         for(const auto e : entities) {
//             auto& collider = coordinator.getComponent<Collider>(e);
//             auto& color = coordinator.getComponent<ColorComponent>(e);
//             for(auto poly : collider.transformed_shape) {
//                 auto prev = poly.back();
//                 sf::Vertex verts[3];
//                 verts[2].position = poly.front();
//                 verts[0].color = color.fill_color;
//                 verts[1].color = color.fill_color;
//                 verts[2].color = color.fill_color;
//                 for(auto p : poly) {
//                     verts[0].position = prev;
//                     verts[1].position = p;
//                     prev = p;
//                     target.draw(verts, 3U, sf::Triangles);
//                 }
//             }
//         }
//     }
// };
//
// class Demo : public TempApp {
//     public:
//     std::shared_ptr<DrawingSystem> drawing_sys;
//
//     std::shared_ptr<PhysicsSystem> physics_sys;
//     std::shared_ptr<RigidbodySystem> rigidbody_sys;
//     std::shared_ptr<ColliderSystem> collider_sys;
//     std::shared_ptr<TransformSystem> transform_sys;
//     std::vector<vec2f> points;
//     vec2f center = window_size  / 2.f;
//     static constexpr float radius = 200;
//     static constexpr float gravity = 500.f * 8.f;
//     Entity createBody(std::vector<vec2f> point_cloud, float density = 1.f) {
//         if(!isTriangulable(point_cloud)) {
//             std::reverse(point_cloud.begin(), point_cloud.end());
//         }
//         auto e = coordinator.createEntity();
//         EMP_LOG_DEBUG << "new entity: " << e;
//         Transform trans(calculateMassInertiaArea(point_cloud).centroid);
//         Collider col(point_cloud, true);
//         Rigidbody rb; rb.prev_pos = trans.position; rb.useAutomaticMass = true;
//         Material mat;
//         
//         float t = ((double) rand() / (RAND_MAX)) + 1;
//         float tt = ((double) rand() / (RAND_MAX)) + 1;
//         float ttt = ((double) rand() / (RAND_MAX)) + 1;
//         const unsigned char min_grey = 50;
//         const unsigned char max_grey = 200;
//         const unsigned char diff = max_grey - min_grey;
//         ColorComponent color; color.fill_color = sf::Color(min_grey + diff * t, min_grey + diff * tt, min_grey + diff * ttt);
//
//         coordinator.addComponent(e, color); 
//         coordinator.addComponent(e, trans); 
//         coordinator.addComponent(e, col); 
//         coordinator.addComponent(e, rb); 
//         coordinator.addComponent(e, mat); 
//         return e;
//     }
//     bool setup(sf::Window& window) override {
//         coordinator.init();
//
//         coordinator.registerComponent<Transform>();
//         coordinator.registerComponent<Material>();
//         coordinator.registerComponent<Collider>();
//         coordinator.registerComponent<Rigidbody>();
//
//         coordinator.registerComponent<ColorComponent>();
//
//
//         transform_sys = coordinator.registerSystem<TransformSystem>();
//         rigidbody_sys = coordinator.registerSystem<RigidbodySystem>();
//         collider_sys  = coordinator.registerSystem<ColliderSystem>();
//         physics_sys   = coordinator.registerSystem<PhysicsSystem>();
//
//         drawing_sys   = coordinator.registerSystem<DrawingSystem>();
//
//         Entity platform = createBody({window_size, vec2f(10.f, window_size.y), vec2f(10.f, window_size.y - 50.f),
//                                    vec2f(window_size.x, window_size.y - 50.f)}, INFINITY);
//         coordinator.getComponent<Rigidbody>(platform).isStatic = true;
//
//         physics_sys->gravity = 500.f;
//
//         return true;
//     }
//
//     void update(sf::Time dt) override {
//         if(keys[sf::Keyboard::P].pressed) {
//             points.push_back(mouse_position);
//             EMP_LOG_DEBUG << "added point";
//         }
//         if(keys[sf::Keyboard::R].pressed) {
//             points.clear();
//             EMP_LOG_DEBUG << "points reset";
//         }
//         if(keys[sf::Keyboard::Enter].pressed) {
//             EMP_LOG_DEBUG << "created body from points";
//             auto body = createBody(points);
//             points.clear();
//         }
//         if(keys[sf::Keyboard::Tab].pressed) {
//             auto body = createBody(points, INFINITY);
//             points.clear();
//         }
//         transform_sys->update();
//         collider_sys->update();
//         physics_sys->update(*transform_sys, *collider_sys, *rigidbody_sys, dt.asSeconds());
//
//     }
//     void render(sf::RenderWindow& window) override {
//         const float point_radius = 5.f;
//         sf::CircleShape cs(point_radius);
//         cs.setFillColor(sf::Color::Red);
//         cs.setOrigin(point_radius, point_radius);
//         for(auto p : points ) {
//             cs.setPosition(p);
//             window.draw(cs);
//         }
//         drawing_sys->draw(window);
//         EMP_DEBUGCALL(
//             cs.setFillColor(sf::Color::Transparent);
//             cs.setOutlineColor(sf::Color::Green);
//             cs.setOutlineThickness(1.f);
//             for(auto p : physics_sys->debug_contactpoints) {
//                 cs.setPosition(p);
//                 window.draw(cs);
//             }
//         );
//     }
// };

int main()
{
    {
        App demo;
        demo.run();
    }
    return 0;
}

