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
        bool isProtagonistGrounded;
        float isProtagonistGroundedSec;
        static constexpr float cayote_time = 0.25f;
        static constexpr float cube_side_len = 35.f;
        std::vector<vec2f> unit_cube = {
                vec2f(-1.f/2.f, -1.f/2.f),
                vec2f(-1.f/2.f, 1.f/2.f),
                vec2f(1.f/2.f, 1.f/2.f),
                vec2f(1.f/2.f, -1.f/2.f)
        };
        std::vector<vec2f> cube_model_shape = {
                vec2f(-cube_side_len/2, -cube_side_len/2),
                vec2f(-cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, -cube_side_len/2)
        };
        static constexpr int markers_count = 64;
        Entity markers[markers_count];
        Entity last_created_crate = 0;
        DebugShape debug_cube_shape = DebugShape(
                device,
                cube_model_shape,
                glm::vec4(1, 0, 0, 1),
                glm::vec4(0, 0, 1, 1)
        );
        void setupAnimationForProtagonist();
        void onRender(Device&, const FrameInfo& frame) override final;
        void onSetup(Window& window, Device& device) override final;
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final;
        void onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller)override final;

        Demo()
            : App({{"../assets/models/colored_cube.obj", "cube"}},
                {
                    {"../assets/textures/dummy.png", "dummy"},
                    {"../assets/textures/crate.jpg", "crate"},
                    {"../assets/textures/knight/_Run.png", "running"},
                    {"../assets/textures/knight/_Jump.png", "jump-up"},
                    {"../assets/textures/knight/_Fall.png", "jump-down"},
                    {"../assets/textures/knight/_Idle.png", "idle"},
                    {"../assets/textures/knight/_JumpFallInbetween.png", "jumpfall"}
                }) {}
};
void Demo::onSetup(Window& window, Device& device) {
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

    coordinator.getSystem<ColliderSystem>()->onCollisionEnter(
            protagonist, protagonist, [&](const CollisionInfo& info) {
                EMP_LOG_DEBUG << "protagonist hit: " << info.collidee_entity;
                isProtagonistGrounded = true;
            }
    ).onCollisionExit(protagonist, protagonist, [&](Entity me, Entity other) {
            EMP_LOG_DEBUG << "protagonist ended hitting: " << other;
            isProtagonistGrounded = false;
        });

    // coordinator.addComponent(cube, Model("cube"));
    protagonist = coordinator.createEntity();
    mouse_entity = coordinator.createEntity();
    coordinator.addComponent(mouse_entity, Transform({0.f, 0.f}));

    {
        Rigidbody rb;
        rb.useAutomaticMass = true;
        rb.isRotationLocked = true;
        coordinator.addComponent(protagonist, Transform(vec2f(), 0.f));
        std::vector<vec2f> protagonist_shape = {
                vec2f(-100.f/4, -100.f/1.5),
                vec2f(-100.f/4, 100.f/1.5),
                vec2f(100.f/4, 100.f/1.5),
                vec2f(100.f/4, -100.f/1.5)
        };
        auto col = Collider(protagonist_shape);
        col.collider_layer = PLAYER;
        // col.listen(protagonist, [&](const CollisionInfo& ci) {
        //     if(dot(ci.collision_normal, vec2f(0.f, -1.f)) > 0.5f) {
        //         isProtagonistGroundedSec = 0.f;
        //     }
        // });

        coordinator.addComponent(protagonist, col);
        coordinator.addComponent(protagonist, rb);

        auto mat = Material(); 
        coordinator.addComponent(protagonist, mat);

        auto db_shape = DebugShape(device, protagonist_shape, glm::vec4(1, 1, 1, 1));
        coordinator.addComponent(protagonist, db_shape);


        setupAnimationForProtagonist();
    }

    std::pair<vec2f, float> ops[4] = {
            {vec2f(0.f, 400.0f), 0.f},
            {vec2f(400.f, 0.0f), M_PI / 2.f},
            {vec2f(0.f, -400.0f), 0.f},
            {vec2f(-400.f, 0.0f), M_PI / 2.f}
    };
    debug_cube_shape.fill_color = glm::vec4(1, 1, 1, 1);
    for (auto o : ops) {
        auto platform = coordinator.createEntity();
        coordinator.addComponent(
                platform, Transform(o.first, o.second, {width / cube_side_len, 1.f})
        );
        coordinator.addComponent(
                platform, DebugShape(device, cube_model_shape)
        );
        auto col = Collider(cube_model_shape);
        col.collider_layer = GROUND;
        coordinator.addComponent(platform, col);
        coordinator.addComponent(platform, Rigidbody{true});
        coordinator.addComponent(platform, Material());
    }
    for (auto& marker : markers) {
        marker = coordinator.createEntity();
        coordinator.addComponent(marker, Transform(vec2f(INFINITY, INFINITY), 0.f, {0.1f, 0.1f}));
        coordinator.addComponent(marker, DebugShape(device, unit_cube, glm::vec4(0.8, 0.8, 1, 1)));
    }
    debug_cube_shape.fill_color = glm::vec4(1, 1, 0, 1);

    coordinator.getSystem<PhysicsSystem>()->gravity = 1000.f;
    coordinator.getSystem<PhysicsSystem>()->substep_count = 8;
}

void Demo::setupAnimationForProtagonist() {
    auto def_size = vec2f(300.f, 300.f);
    auto offset = vec2f(10.f, -def_size.y / 3.5f);
    MovingSprite idle_moving;
    {
        Sprite idle_sprite = Sprite(Texture("idle"), def_size);
        idle_sprite.position_offset = offset;
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
        {
            Sprite running_sprite = Sprite(Texture("running"), def_size);
            running_sprite.position_offset = offset;
            running_sprite.centered = true;
            running_sprite.hframes = 10;
            running_sprite.vframes = 1;
            build.addNode("run",  MovingSprite::allFrames(running_sprite, 0.6f));
        }
        {
            Sprite jumping_spr = Sprite(Texture("jump-up"), def_size);
            jumping_spr.position_offset = offset;
            jumping_spr.centered = true;
            jumping_spr.hframes = 3;
            jumping_spr.vframes = 1;
            build.addNode("jump", MovingSprite::allFrames(jumping_spr, 0.25f, false));
        }
        {
            Sprite jumpfall_spr = Sprite(Texture("jumpfall"), def_size);
            jumpfall_spr.position_offset = offset;
            jumpfall_spr.centered = true;
            jumpfall_spr.hframes = 2;
            jumpfall_spr.vframes = 1;
            build.addNode("jumpfall", MovingSprite::allFrames(jumpfall_spr, 0.25f, false));
        }
        {
            Sprite falling_spr = Sprite(Texture("jump-down"), def_size);
            falling_spr.position_offset = offset;
            falling_spr.centered = true;
            falling_spr.hframes = 3;
            falling_spr.vframes = 1;
            build.addNode("fall", MovingSprite::allFrames(falling_spr, 0.25f, false));
        }

        build.addEdge("idle", "run", [](Entity owner) {
            return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) >
                   25.f;
        });
        build.addEdge("run", "idle", [](Entity owner, bool isended) {
            return abs(coordinator.getComponent<Rigidbody>(owner)->velocity.x) <
                   25.f;
        });
        auto isJumping = [](Entity owner) {
            return coordinator.getComponent<Rigidbody>(owner)->velocity.y <
                   -100.f;
        };
        auto isFalling = [](Entity owner) {
            return coordinator.getComponent<Rigidbody>(owner)->velocity.y >
                   25.f;
        };
        auto hasFallen = [&](Entity owner) {
            return isProtagonistGrounded == true;
        };
        build.addEdge("idle", "fall", isFalling);
        build.addEdge("run", "fall", isFalling);
        build.addEdge("idle", "jump", isJumping);
        build.addEdge("run", "jump", isJumping);
        build.addEdge("jump", "jumpfall", isFalling);
        build.addEdge("jumpfall", "fall");
        build.addEdge("fall", "idle", hasFallen);
    }
    auto anim_sprite = AnimatedSprite(build);
    coordinator.addComponent(protagonist, anim_sprite);
}
void Demo::onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller) {
}
void Demo::onRender(Device&, const FrameInfo& frame) {
    ImGui::ShowDemoWindow();
}
void Demo::onUpdate(const float delta_time, Window& window, KeyboardController& controller) 
{
    auto& phy_sys = *coordinator.getSystem<PhysicsSystem>();
    for(auto e : phy_sys.entities) {
        if(phy_sys.m_isDormant(e)) {
            coordinator.getComponent<DebugShape>(e)->fill_color = {1, 0, 0, 1};
        }else {
            coordinator.getComponent<DebugShape>(e)->fill_color = {1, 1, 1, 1};
        }
    }
    for(auto m : markers) {
        coordinator.getComponent<Transform>(m)->position = {INFINITY, INFINITY};
        coordinator.getComponent<Transform>(m)->scale = {1, 1};
    }
    int i = 0;
    for(auto entity : phy_sys.entities) {
        if(i == markers_count) {
            break;
        }
        auto& dis_set = phy_sys.m_collision_islands;
        auto other = dis_set.group(entity);

        auto& other_trans = *coordinator.getComponent<Transform>(other);
        auto& this_trans = *coordinator.getComponent<Transform>(entity);
        vec2f pos1 = this_trans.position;
        vec2f pos2 = other_trans.position;

        auto& shape = *coordinator.getComponent<Transform>(markers[i++]);
        shape.position = (pos1 + pos2) * 0.5f;
        shape.scale = vec2f(1.f, length(pos1 - pos2));
        shape.rotation = angle(vec2f(0.f, 1.f), pos1 - pos2);
    }

    {
        coordinator.getComponent<Transform>(mouse_entity)->position =
                controller.global_mouse_pos();
    }
    if(coordinator.isEntityAlive(protagonist)){
        isProtagonistGroundedSec += delta_time;
        if(isProtagonistGrounded) {
            isProtagonistGroundedSec = 0.f;
        } 
        if(coordinator.isEntityAlive(protagonist)){
            coordinator.getComponent<Rigidbody>(protagonist)->velocity.x +=
                    controller.movementInPlane2D().x * 600.f * delta_time;
            auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
            if (controller.get(eKeyMappings::Jump).pressed && isProtagonistGroundedSec < cayote_time) {
                isProtagonistGroundedSec = cayote_time;
                isProtagonistGrounded = false;
                vec2f direction = vec2f(0, -10);
                direction.x = controller.movementInPlane2D().x;
                direction = normal(direction);
                rb.velocity += 500.f * direction;
            }
        }
        auto& rb = *coordinator.getComponent<Rigidbody>(protagonist);
        auto& trans = *coordinator.getComponent<Transform>(protagonist);
        bool isRunningLeft = rb.velocity.x < 0.f;
        if(abs(rb.velocity.x) > 10.f) {
            trans.scale.x = isRunningLeft ?  -1.f : 1.f;
        }
    }

    static vec2f last_pos;
    if (controller.get(eKeyMappings::Ability1).held && last_pos != vec2f(controller.global_mouse_pos())) {
        last_pos = vec2f(controller.global_mouse_pos());
        Sprite spr = Sprite(Texture("crate"), {cube_side_len, cube_side_len});
        Rigidbody rb;
        rb.useAutomaticMass = true;
        auto col = Collider(cube_model_shape);
        col.collider_layer = ITEM;
        auto entity = coordinator.createEntity();
        last_created_crate = entity;
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
}

