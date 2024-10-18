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
        static constexpr float cube_side_len = 100.f;
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
                    {"../assets/textures/idle.png", "idle"}
                }) {}
};
void Demo::onSetup(Window& window, Device& device) {
    Collider::disableCollision(PLAYER, ITEM);
    controller.bind(eKeyMappings::Ability1, GLFW_KEY_C);
    controller.bind(eKeyMappings::Jump, GLFW_KEY_SPACE);

    controller.bind(eKeyMappings::LookUp, GLFW_KEY_W);
    controller.bind(eKeyMappings::LookDown, GLFW_KEY_S);
    controller.bind(eKeyMappings::LookLeft, GLFW_KEY_D);
    controller.bind(eKeyMappings::LookRight, GLFW_KEY_A);

    controller.bind(eKeyMappings::MoveUp, GLFW_KEY_UP);
    controller.bind(eKeyMappings::MoveDown, GLFW_KEY_DOWN);
    controller.bind(eKeyMappings::MoveLeft, GLFW_KEY_LEFT);
    controller.bind(eKeyMappings::MoveRight, GLFW_KEY_RIGHT);

    // coordinator.addComponent(cube, Model("cube"));
    protagonist = coordinator.createEntity();
    mouse_entity = coordinator.createEntity();
    coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

    Sprite spr = Sprite(Texture("jump-down"), {cube_side_len, cube_side_len});
    Rigidbody rb;
    rb.useAutomaticMass = true;
    rb.isRotationLocked = true;
    coordinator.addComponent(protagonist, Transform(vec2f(), 0.f));
    debug_cube_shape.fill_color = glm::vec4(1, 1, 1, 1);
    coordinator.addComponent(protagonist, debug_cube_shape);
    debug_cube_shape.fill_color = glm::vec4(1, 1, 0, 1);
    auto col = Collider(cube_model_shape);
    col.collider_layer = PLAYER;
    coordinator.addComponent(protagonist, col);
    coordinator.addComponent(protagonist, rb);
    auto mat = Material(); mat.dynamic_friction *= 2.f;
    coordinator.addComponent(protagonist, mat);

    auto def_size = vec2f(cube_side_len * 2.f, cube_side_len * 2.f);
    MovingSprite idle_moving;
    {
        Sprite idle_sprite = Sprite(Texture("idle"), def_size);
        idle_sprite.position_offset = {0.f, -cube_side_len / 2.f};
        idle_sprite.centered = true;
        idle_sprite.hframes = 10;
        idle_sprite.vframes = 1;
        idle_moving.sprite = idle_sprite;
        for(int i = 0; i < idle_sprite.frameCount(); i++) {
            idle_moving.add(i, 0.085f);
        }
    }
    AnimatedSprite::Builder build("idle", idle_moving);
    {
        Sprite running_sprite = Sprite(Texture("running"), def_size);
        running_sprite.position_offset = {0.f, -cube_side_len / 2.f};
        running_sprite.centered = true;
        running_sprite.hframes = 10;
        running_sprite.vframes = 1;
        MovingSprite moving;
        moving.sprite = running_sprite;
        for(int i = 0; i < running_sprite.frameCount(); i++) {
            moving.add(i, 0.085f);
        }
        build.addNode("run",  moving);
        build.addNode("jump", MovingSprite::singleFrame(Sprite(Texture("jump-up"), def_size)));
        build.addNode("fall", MovingSprite::singleFrame(Sprite(Texture("jump-down"), def_size)));

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
    if(coordinator.isEntityAlive(protagonist)){
        coordinator.getComponent<Rigidbody>(protagonist)->velocity.x +=
                controller.movementInPlane2D().x * 600.f * delta_time;
        auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
        auto& animated = *coordinator.getComponent<AnimatedSprite>(protagonist);
        bool isRunningLeft = rb.velocity.x < 0.f;
        if(abs(rb.velocity.x) > 10.f) {
            animated.flipX = isRunningLeft;
        }
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

