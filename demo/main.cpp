#include "physics/constraint.hpp"
#include "scene/app.hpp"
#include "graphics/sprite_system.hpp"
using namespace emp;

class Demo : public App {
    public:
        Entity mouse_entity;
        Entity cube;
        std::vector<vec2f> cube_model_shape = {vec2f(-50.0f, -50.0f), vec2f(-50.0f, 50.0f), vec2f(50.0f, 50.0f), vec2f(50.0f, -50.0f)};
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final {
            {
                coordinator.getComponent<Transform>(mouse_entity)->position = controller.global_mouse_pos();
            }
            if(controller.get(eKeyMappings::Ability1).pressed) {
                Rigidbody rb; rb.useAutomaticMass = true;
                auto e = coordinator.createEntity();
                coordinator.addComponent(e, Transform(vec2f(controller.global_mouse_pos()), 0.f));
                coordinator.addComponent(e, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
                coordinator.addComponent(e, Collider(cube_model_shape)); 
                coordinator.addComponent(e, rb); 
                coordinator.addComponent(e, Material()); 
                coordinator.addComponent(e, SpriteRenderer("test"));
            }
        }
        void onRender(Device&, const FrameInfo& frame) override final {
        }
        void onSetup(Window& window, Device& device) override final {
            controller.bind(eKeyMappings::Ability1, GLFW_KEY_SPACE);
            // coordinator.addComponent(cube, Model("cube"));
            cube = coordinator.createEntity();
            mouse_entity = coordinator.createEntity();
            coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

            Sprite::create("test", Texture("dummy"), {{0, 0}, {1, 1}});
            auto rend = SpriteRenderer("test");
            rend.flipX = true;
            // coordinator.addComponent(cube, rend);

            
            for(int ii = 0; ii < 0; ii++)
            for(int i = 0; i < 0; i++) {
                auto e = coordinator.createEntity();
                coordinator.addComponent(e, Transform(vec2f(i, ii), 0.f));
                coordinator.addComponent(e, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
                coordinator.addComponent(e, Collider(cube_model_shape)); 
                coordinator.addComponent(e, Rigidbody()); 
                coordinator.addComponent(e, Material()); 
                coordinator.addComponent(e, rend);
            }

            // Constraint constraint;
            // constraint = Constraint::createPointAnchor(mouse_entity, cube, vec2f(0.f, cube_scale));
            // constraint.compliance = 0.001f;
            // auto spring = coordinator.createEntity();
            // coordinator.addComponent(spring, constraint);


            auto platform = coordinator.createEntity();
            coordinator.addComponent(platform, Transform(vec2f(0.f, 0.0f), 0.f, {8.0f, 1.f}));
            coordinator.addComponent(platform, DebugShape(device, cube_model_shape));
            coordinator.addComponent(platform, Collider(cube_model_shape)); 
            coordinator.addComponent(platform, Rigidbody{true}); 
            coordinator.addComponent(platform, Material()); 

            coordinator.getSystem<PhysicsSystem>()->gravity = 1000.f;
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

