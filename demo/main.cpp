#include "graphics/animated_sprite.hpp"
#include "physics/constraint.hpp"
#include "scene/app.hpp"
#include "graphics/sprite_system.hpp"
#include "templates/finite_state_machine.hpp"
using namespace emp;

class Demo : public App {
    public:
        Entity mouse_entity;
        Entity protagonist;
        std::vector<vec2f> cube_model_shape = {vec2f(-50.0f, -50.0f), vec2f(-50.0f, 50.0f), vec2f(50.0f, 50.0f), vec2f(50.0f, -50.0f)};
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final {
            {
                coordinator.getComponent<Transform>(mouse_entity)->position = controller.global_mouse_pos();
            }
            {
                coordinator.getComponent<Rigidbody>(protagonist)->vel.x += controller.movementInPlane2D().x * 10.f;
            }

            if(controller.get(eKeyMappings::Ability1).pressed) {
                Sprite spr = Sprite(Texture("running"), {100.f, 100.f});
                spr.hframes = 5;
                spr.vframes = 2;
                spr.frame = 9;
                Rigidbody rb; rb.useAutomaticMass = true;
                auto e = coordinator.createEntity();
                coordinator.addComponent(e, Transform(vec2f(controller.global_mouse_pos()), 0.f));
                coordinator.addComponent(e, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
                coordinator.addComponent(e, Collider(cube_model_shape)); 
                coordinator.addComponent(e, rb); 
                coordinator.addComponent(e, Material()); 
                coordinator.addComponent(e, spr);
            }
            coordinator.removeComponent<Sprite>(protagonist);
            auto& anim = *coordinator.getComponent<AnimatedSprite>(protagonist);
            anim.updateState(protagonist);
            coordinator.addComponent<Sprite>(protagonist, anim.sprite());
        }
        void onRender(Device&, const FrameInfo& frame) override final {
        }
        void onSetup(Window& window, Device& device) override final {
            controller.bind(eKeyMappings::Ability1, GLFW_KEY_SPACE);
            // coordinator.addComponent(cube, Model("cube"));
            protagonist = coordinator.createEntity();
            mouse_entity = coordinator.createEntity();
            coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

            
            Sprite spr = Sprite(Texture("jump-down"), {100.f, 100.f});
            Rigidbody rb; rb.useAutomaticMass = false;
            rb.real_inertia = INFINITY;
            rb.real_mass = 100 * 100 * 100;
            coordinator.addComponent(protagonist, Transform(vec2f(), 0.f));
            coordinator.addComponent(protagonist, DebugShape(device, cube_model_shape, glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 1, 1)));
            coordinator.addComponent(protagonist, Collider(cube_model_shape)); 
            coordinator.addComponent(protagonist, rb); 
            coordinator.addComponent(protagonist, Material()); 
            coordinator.addComponent(protagonist, spr);
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
            spr.hframes = 5;
            spr.vframes = 2;
            spr.frame = 9;
            build.addNode("run", running_sprite); 
            build.addNode("jump", Sprite(Texture("jump-up"), def_size)); 
            build.addNode("fall", Sprite(Texture("jump-down"), def_size)); 

            build.addEdge("idle", "run", [](Entity owner) {
                return abs(coordinator.getComponent<Rigidbody>(owner)->vel.x) > 0.f;
            });
            build.addEdge("run", "idle", [](Entity owner) {
                return abs(coordinator.getComponent<Rigidbody>(owner)->vel.x) == 0.f;
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
                  }) {
        }
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

