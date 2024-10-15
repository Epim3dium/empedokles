#include "graphics/animated_sprite.hpp"
#include "scene/app.hpp"
using namespace emp;

class Demo : public App {
    public:
        Entity mouse_entity;
        Entity protagonist;
        std::vector<vec2f> cube_model_shape = {vec2f(-50.0f, -50.0f), vec2f(-50.0f, 50.0f), vec2f(50.0f, 50.0f), vec2f(50.0f, -50.0f)};
        DebugShape debug_cube_shape = DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1));
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final {
            {
                coordinator.getComponent<Transform>(mouse_entity)->position = controller.global_mouse_pos();
            }
            {
                coordinator.getComponent<Rigidbody>(protagonist)->velocity.x += controller.movementInPlane2D().x * 600.f * delta_time;
                auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
                auto& animated = *coordinator.getComponent<AnimatedSprite>(protagonist);
                bool isRunningLeft = rb.velocity.x < 0.f;
                animated.flipX = isRunningLeft;
            }

            if(controller.get(eKeyMappings::Ability1).pressed) {
                Sprite spr = Sprite(Texture("dummy"), {100.f, 100.f});
                Rigidbody rb; rb.useAutomaticMass = true;
                auto e = coordinator.createEntity();
                coordinator.addComponent(e, Transform(vec2f(controller.global_mouse_pos()), 0.f));
                coordinator.addComponent(e, debug_cube_shape);
                coordinator.addComponent(e, Collider(cube_model_shape)); 
                coordinator.addComponent(e, rb); 
                coordinator.addComponent(e, Material()); 
                coordinator.addComponent(e, spr);
            }
        }
        void onRender(Device&, const FrameInfo& frame) override final {
        }
        void onSetup(Window& window, Device& device) override final {
            controller.bind(eKeyMappings::Ability1, GLFW_KEY_E);
            controller.bind(eKeyMappings::Jump, GLFW_KEY_SPACE);
            // coordinator.addComponent(cube, Model("cube"));
            protagonist = coordinator.createEntity();
            mouse_entity = coordinator.createEntity();
            coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

            
            Sprite spr = Sprite(Texture("jump-down"), {100.f, 100.f});
            Rigidbody rb; rb.useAutomaticMass = true;
            rb.isRotationLocked = true;
            coordinator.addComponent(protagonist, Transform(vec2f(), 0.f));
            coordinator.addComponent(protagonist, debug_cube_shape);
            coordinator.addComponent(protagonist, Collider(cube_model_shape)); 
            coordinator.addComponent(protagonist, rb); 
            coordinator.addComponent(protagonist, Material()); 
            // for(int ii = 0; ii < 0; ii++)
            // for(int i = 0; i < 0; i++) {
            //     auto e = coordinator.createEntity();
            //     coordinator.addComponent(e, Transform(vec2f(i, ii), 0.f));
            //     coordinator.addComponent(e, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
            //     coordinator.addComponent(e, Collider(cube_model_shape)); 
            //     coordinator.addComponent(e, Rigidbody()); 
            //     coordinator.addComponent(e, Material()); 
            //     coordinator.addComponent(e, spr);
            // }


            auto platform = coordinator.createEntity();

            auto def_size = vec2f(100.f, 100.f);
            AnimatedSprite::Builder build("idle", Sprite(Texture("idle"), def_size));
            Sprite running_sprite = Sprite(Texture("running"), {100.f, 100.f});
            running_sprite.hframes = 5;
            running_sprite.vframes = 2;
            running_sprite.frame = 9;
            build.addNode("run", running_sprite); 
            build.addNode("jump", Sprite(Texture("jump-up"), def_size)); 
            build.addNode("fall", Sprite(Texture("jump-down"), def_size)); 

            build.addEdge("idle", "run", [](Entity owner) {
                return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) > 10.f;
            });
            build.addEdge("run", "idle", [](Entity owner) {
                return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) < 10.f;
            });
            coordinator.addComponent(protagonist, AnimatedSprite(build));

            coordinator.addComponent(platform, Transform(vec2f(0.f, 500.0f), 0.f, {8.0f, 1.f}));
            coordinator.addComponent(platform, DebugShape(device, cube_model_shape));
            coordinator.addComponent(platform, Collider(cube_model_shape)); 
            coordinator.addComponent(platform, Rigidbody{true}); 
            coordinator.addComponent(platform, Material()); 

            coordinator.getSystem<PhysicsSystem>()->gravity = 1000.f;
            // coordinator.addComponent(cube, Texture("default"));
            EMP_LOG(LogLevel::DEBUG) << "cube created, id: " << protagonist;
            {
            }
        }
        Demo()
            : App({{"../assets/models/colored_cube.obj", "cube"}},
                  {
                  {"../assets/textures/dummy.png", "dummy"},
                  {"../assets/textures/running.png", "running"},
                  {"../assets/textures/jumping.png", "jump-up"},
                  {"../assets/textures/falling.png", "jump-down"},
                  {"../assets/textures/standing.png", "idle"}
              }) {}
};
int main()
{
    {
        Demo demo;
        demo
            .run();
    }
    return 0;
}

