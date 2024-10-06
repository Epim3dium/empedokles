#include "physics/constraint.hpp"
#include "scene/app.hpp"
#include "graphics/sprite_system.hpp"
using namespace emp;

class Demo : public App {
    public:
        Entity mouse_entity;
        Entity cube;
        void onUpdate(const float delta_time, Window& window) override final {
            double xpos, ypos;
            {
                glfwGetCursorPos(window.getGLFWwindow(), &xpos, &ypos);
                auto xdenom = static_cast<float>(window.getExtent().width);
                auto ydenom = static_cast<float>( window.getExtent().height);
                xpos = (xpos + (ydenom - xdenom) / 2.f) / ydenom * 2.f - 1.f;
                ypos = ypos / ydenom * 2.f - 1.f;
                coordinator.getComponent<Transform>(mouse_entity)->position = {xpos, ypos};
            }
        }
        void onRender(Device&, const FrameInfo& frame) override final {
        }
        std::vector<vec2f> cube_model_shape = {vec2f(-1.0f, -1.0f), vec2f(-1.0f, 1.0f), vec2f(1.0f, 1.0f), vec2f(1.0f, -1.0f)};
        void onSetup(Window& window, Device& device) override final {
            // coordinator.addComponent(cube, Model("cube"));
            cube = coordinator.createEntity();
            mouse_entity = coordinator.createEntity();
            coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

            Sprite::create("test", Texture("dummy"), {{0, 0}, {1, 1}});
            auto rend = SpriteRenderer("test");
            rend.flipX = true;
            coordinator.addComponent(cube, rend);

            const float cube_scale = 0.125f; 
            coordinator.addComponent(cube, Transform(vec2f(0.f, 0.f), 0.f, {cube_scale, cube_scale}));
            coordinator.addComponent(cube, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
            coordinator.addComponent(cube, Collider(cube_model_shape)); 
            Rigidbody rb; rb.useAutomaticMass = false;
            rb.real_mass = 0.1f; rb.real_inertia = 1.f / 6.f * 0.1f;
            coordinator.addComponent(cube, rb); 
            coordinator.addComponent(cube, Material()); 

            Constraint constraint;
            constraint = Constraint::createPointAnchor(mouse_entity, cube, vec2f(0.f, cube_scale));
            constraint.compliance = 0.001f;
            auto spring = coordinator.createEntity();
            coordinator.addComponent(spring, constraint);


            auto platform = coordinator.createEntity();
            coordinator.addComponent(platform, Transform(vec2f(0.f, 1.0f), 0.f, {1.0f, 0.125f}));
            coordinator.addComponent(platform, DebugShape(device, cube_model_shape));
            coordinator.addComponent(platform, Collider(cube_model_shape)); 
            coordinator.addComponent(platform, Rigidbody{true}); 
            coordinator.addComponent(platform, Material()); 

            coordinator.getSystem<PhysicsSystem>()->gravity = 10.f;
            // coordinator.addComponent(cube, Texture("default"));
            EMP_LOG(LogLevel::DEBUG) << "cube created, id: " << cube;
            {
            }
        }
        Demo() : App({{"../assets/models/colored_cube.obj", "cube"}}, {{"../assets/textures/dummy.png", "dummy"}}) {}
};
int main()
{
    // coordinator.init();
    // LooseTightDoubleGrid grid(AABB::CreateMinMax({-1000.f, -1'000.f}, {1'000.f, 1'000.f}), vec2f(1.f, 1.f));
    // Entity obj1 = coordinator.createEntity();
    // AABB obj1rect = AABB::CreateMinMax({-1.f, -1.f}, {1.f, 1.f});
    // grid.insert(obj1rect, obj1);
    // EMP_LOG_DEBUG << "created and inserted: " << obj1;
    //
    // Entity obj2 = coordinator.createEntity();
    // AABB obj2rect = AABB::CreateMinMax({0.5f, 0.f}, {1.5f, 10.f});
    // grid.insert(obj2rect, obj2);
    // EMP_LOG_DEBUG << "created and inserted: " << obj2;
    //
    // for(auto e : grid.query(obj1rect)) {
    //     EMP_LOG_DEBUG << e;
    // }
    //
    //
    // exit(1);
    {
        Demo demo;
        demo
            .run();
    }
    return 0;
}

