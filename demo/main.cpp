#include "core/layer.hpp"
#include "graphics/animated_sprite.hpp"
#include "scene/app.hpp"
using namespace emp;

enum CollisionLayers {
    GROUND,
    PLAYER,
    ITEM
};
class Demo : public App {
    public:
        Entity mouse_entity;
        Entity protagonist;
        static constexpr float cube_side_len = 50.f;
        std::vector<vec2f> cube_model_shape = {
                vec2f(-cube_side_len/2, -cube_side_len/2),
                vec2f(-cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, -cube_side_len/2)
        };
        DebugShape debug_cube_shape = DebugShape(
                device,
                cube_model_shape,
                glm::vec4(1, 0, 0, 1),
                glm::vec4(0, 0, 1, 1)
        );
        void onRender(Device&, const FrameInfo& frame) override final { }
        void onSetup(Window& window, Device& device) override final;
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final;
        
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
void Demo::onSetup(Window& window, Device& device) {
    Collider::disableCollision(PLAYER, ITEM);
    controller.bind(eKeyMappings::Ability1, GLFW_KEY_C);
    controller.bind(eKeyMappings::Jump, GLFW_KEY_SPACE);
    // coordinator.addComponent(cube, Model("cube"));
    protagonist = coordinator.createEntity();
    mouse_entity = coordinator.createEntity();
    coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

    Sprite spr = Sprite(Texture("jump-down"), {cube_side_len, cube_side_len});
    Rigidbody rb;
    rb.useAutomaticMass = true;
    rb.isRotationLocked = true;
    coordinator.addComponent(protagonist, Transform(vec2f(), 0.f));
    coordinator.addComponent(protagonist, debug_cube_shape);
    auto col = Collider(cube_model_shape);
    col.collider_layer = PLAYER;
    coordinator.addComponent(protagonist, col);
    coordinator.addComponent(protagonist, rb);
    coordinator.addComponent(protagonist, Material());

    auto def_size = vec2f(cube_side_len, cube_side_len);
    AnimatedSprite::Builder build("idle", Sprite(Texture("idle"), def_size));
    {
        Sprite running_sprite = Sprite(Texture("running"), {cube_side_len, cube_side_len});
        running_sprite.hframes = 5;
        running_sprite.vframes = 2;
        running_sprite.frame = 9;
        build.addNode("run", running_sprite);
        build.addNode("jump", Sprite(Texture("jump-up"), def_size));
        build.addNode("fall", Sprite(Texture("jump-down"), def_size));

        build.addEdge("idle", "run", [](Entity owner) {
            return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) >
                   10.f;
        });
        build.addEdge("run", "idle", [](Entity owner) {
            return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) <
                   10.f;
        });
    }
    coordinator.addComponent(protagonist, AnimatedSprite(build));

    std::pair<vec2f, float> ops[4] = {
            {vec2f(0.f, 400.0f), 0.f},
            {vec2f(400.f, 0.0f), M_PI / 2.f},
            {vec2f(0.f, -400.0f), 0.f},
            {vec2f(-400.f, 0.0f), M_PI / 2.f}
    };
    for (auto o : ops) {
        auto platform = coordinator.createEntity();
        coordinator.addComponent(
                platform, Transform(o.first, o.second, {20.0f, 1.f})
        );
        coordinator.addComponent(
                platform, DebugShape(device, cube_model_shape)
        );
        col.collider_layer = GROUND;
        coordinator.addComponent(platform, col);
        coordinator.addComponent(platform, Rigidbody{true});
        coordinator.addComponent(platform, Material());
    }

    coordinator.getSystem<PhysicsSystem>()->gravity = 1000.f;
    // coordinator.addComponent(cube, Texture("default"));
    {
    }
}
void Demo::onUpdate(const float delta_time, Window& window, KeyboardController& controller) 
{
    {
        coordinator.getComponent<Transform>(mouse_entity)->position =
                controller.global_mouse_pos();
    }
    {
        coordinator.getComponent<Rigidbody>(protagonist)->velocity.x +=
                controller.movementInPlane2D().x * 600.f * delta_time;
        auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
        auto& animated = *coordinator.getComponent<AnimatedSprite>(protagonist);
        bool isRunningLeft = rb.velocity.x < 0.f;
        animated.flipX = isRunningLeft;
    }

    if (controller.get(eKeyMappings::Ability1).pressed) {
        Sprite spr = Sprite(Texture("dummy"), {cube_side_len, cube_side_len});
        Rigidbody rb;
        rb.useAutomaticMass = true;
        auto col = Collider(cube_model_shape);
        col.collider_layer = ITEM;
        auto entity = coordinator.createEntity();
        coordinator.addComponent(
                entity, Transform(vec2f(controller.global_mouse_pos()), 0.f)
        );
        coordinator.addComponent(entity, debug_cube_shape);
        coordinator.addComponent(entity, col);
        coordinator.addComponent(entity, rb);
        coordinator.addComponent(entity, Material());
        coordinator.addComponent(entity, spr);
    }
}
int main()
{
    {
        std::cout << sizeof(Collider);
        Demo demo;
        demo
            .run();
    }
    return 0;
}

