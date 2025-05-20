#include "debug/log.hpp"
#include "graphics/animated_sprite.hpp"
#include "graphics/particle_system.hpp"
#include "gui/gui_manager.hpp"
#include "io/keyboard_controller.hpp"
#include "physics/collider.hpp"
#include "physics/rigidbody.hpp"
#include "scene/app.hpp"
#include "utils/time.hpp"
#include <cmath>
#include <cstdio>
#include <vector>
using namespace emp;

class MouseSelectionSystem : public System<Transform, Collider, Rigidbody> {
public:
    std::vector<Entity> query(vec2f point) {
        std::vector<Entity> result;
        for(auto e : entities) {
            auto& shape = getComponent<Collider>(e);
            auto& transform = getComponent<Transform>(e);
            auto transformed_outline = shape.transformed_outline(transform);
            
            auto overlap = isOverlappingPointPoly(point, transformed_outline);
            if(overlap) {
                result.push_back(e);
            }
        }
        return result;
    }
};

enum CollisionLayers {
    GROUND,
    PLAYER,
    FRIENDLY,
    ITEM
};
class Demo : public App {
    public:

        Texture crate_texture;
        Entity mouse_entity;
        Entity protagonist;
        Entity background_static;

        std::vector<vec2f> protagonist_shape = {
                vec2f(-100.f/4, -100.f/1.5),
                vec2f(-100.f/4, 100.f/1.5),
                vec2f(100.f/4, 100.f/1.5),
                vec2f(100.f/4, -100.f/1.5)
        };
        
        float protagonist_speed = 600.f;
        float protagonist_speed_max = 100000.f;
        Entity isProtagonistGrounded;
        bool isProtagonistAttacking;
        bool isProtagonistRolling;
        Stopwatch timerSinceRoll;
        float isProtagonistGroundedSec;
        static constexpr float cayote_time = 0.25f;
        static constexpr float cube_side_len = 70.f;
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
        // static constexpr int markers_count = 64;
        // Entity markers[markers_count];
        Entity last_created_crate = 0;


        void setupAnimationForProtagonist(Entity prot);
        void onRender(Device&, const FrameInfo& frame) override final;
        void onSetup(Window& window, Device& device) override final;
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final;
        void onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller)override final;

        Demo(int w = 1440, int h = 810)
            : App(w, h, {{"../assets/models/colored_cube.obj", "cube"}},
                {
                    {"../assets/textures/dummy.png", "dummy"},
                    {"../assets/textures/crate.jpg", "crate"},
                    {"../assets/textures/background.jpg", "background"},
                    {"../assets/textures/knight/_Run.png", "running"},
                    {"../assets/textures/knight/_Jump.png", "jump-up"},
                    {"../assets/textures/knight/_Fall.png", "jump-down"},
                    {"../assets/textures/knight/_Idle.png", "idle"},
                    {"../assets/textures/knight/_AttackNoMovement.png", "attack"},
                    {"../assets/textures/knight/_Roll.png", "roll"},
                    {"../assets/textures/knight/_JumpFallInbetween.png", "jumpfall"},
                })
                {}
};
void Demo::onSetup(Window& window, Device& device) {
    Log::enableLoggingToCerr();
    gui_manager.alias(ECS.world(), "world_entity");
    ECS.registerSystem<MouseSelectionSystem>();
    ECS.getSystem<ColliderSystem>()->disableCollision(FRIENDLY, FRIENDLY);
    ECS.getSystem<ColliderSystem>()->disableCollision(FRIENDLY, PLAYER);

    crate_texture = Texture("crate");
    controller.bind(eKeyMappings::Shoot, GLFW_MOUSE_BUTTON_LEFT);

    controller.bind(eKeyMappings::Ability1, GLFW_KEY_C);
    controller.bind(eKeyMappings::Ability2, GLFW_KEY_T);
    controller.bind(eKeyMappings::Ability3, GLFW_KEY_H);
    controller.bind(eKeyMappings::Ability4, GLFW_KEY_J);
    controller.bind(eKeyMappings::Jump, GLFW_KEY_SPACE);

    controller.bind(eKeyMappings::LookUp, GLFW_KEY_W);
    controller.bind(eKeyMappings::LookDown, GLFW_KEY_S);
    controller.bind(eKeyMappings::LookLeft, GLFW_KEY_D);
    controller.bind(eKeyMappings::LookRight, GLFW_KEY_A);

    controller.bind(eKeyMappings::MoveUp, GLFW_KEY_UP);
    controller.bind(eKeyMappings::MoveDown, GLFW_KEY_DOWN);
    controller.bind(eKeyMappings::MoveLeft, GLFW_KEY_LEFT);
    controller.bind(eKeyMappings::MoveRight, GLFW_KEY_RIGHT);

    background_static = ECS.createEntity();
    auto spr = Sprite(Texture("background"));
    spr.order = 0.7;
    spr.color = {0.3, 0.3, 0.3, 1};
    ECS.addComponent(background_static, spr);
    ECS.addComponent(background_static, Transform(vec2f(0, 0), 0, vec2f(2, 2)));

    protagonist = ECS.createEntity();
    gui_manager.alias(protagonist, "protagonist");

    ECS.getSystem<ColliderSystem>()
        ->onCollisionEnter(protagonist,
            protagonist,
            [&](const CollisionInfo& info) {
                auto ang = angle(info.collision_normal, vec2f(0, 1));
                if (cos(ang) > -M_PI / 4.f) {
                    return;
                }
                isProtagonistGrounded = info.collidee_entity;
            })
        .onCollisionExit(
            protagonist, protagonist, [&](Entity me, Entity other) {
                if (other == isProtagonistGrounded || isProtagonistGrounded == false) {
                    isProtagonistGrounded = false;
                }
            });

    mouse_entity = ECS.createEntity();
    gui_manager.alias(mouse_entity, "mouse_entity");
    ECS.addComponent(mouse_entity, Transform({0.f, 0.f}));
    ParticleEmitter emitter;
    emitter.enabled = false;
    emitter.colors = {vec4f(1, 0, 0, 1)};
    emitter.lifetime = {0.f, 1.f};
    emitter.count = 50U;
    emitter.setLine({100.f, 0});
    emitter.speed = {1.f, 10.f};
    ECS.addComponent(mouse_entity, emitter);

    {
        Rigidbody rb;
        rb.useAutomaticMass = true;
        rb.isRotationLocked = true;
        ECS.addComponent(protagonist, Transform(vec2f(), 0.f));
        auto col = Collider(protagonist_shape);
        col.collider_layer = PLAYER;

        ECS.addComponent(protagonist, col);
        ECS.addComponent(protagonist, rb);

        auto mat = Material(); 
        ECS.addComponent(protagonist, mat);

        setupAnimationForProtagonist(protagonist);
        auto child = ECS.createEntity();
        gui_manager.alias(child, "child");
        ECS.addComponent(child, Transform(protagonist, vec2f(0, 0)));
    }

    std::pair<vec2f, float> ops[4] = {
            {vec2f(0.f, getHeight() / 2), 0.f},
            {vec2f(getWidth()/ 2, 0.0f), M_PI / 2.f},
            {vec2f(0.f, -getHeight() / 2), 0.f},
            {vec2f(-getWidth() / 2, 0.0f), M_PI / 2.f}
    };
    {
        auto builder = ModelAsset::Builder();
        for(auto v : cube_model_shape) {
            builder.vertices.push_back({});
            builder.vertices.back().position = vec3f(v.x, v.y, 0);
        }
        builder.indices = {0, 1, 2, 2, 3, 0};
        Model::create("platform_debug", device, builder);
    }
    for (int i = 0; i < 4; i++) {
        auto platform = ECS.createEntity();
        gui_manager.alias(platform, "platform");
        ECS.addComponent(
                platform, Transform(ops[i].first, ops[i].second, {getWidth() / cube_side_len * 2, 1.f})
        );

        auto col = Collider(cube_model_shape);
        col.collider_layer = GROUND;
        ECS.addComponent(platform, col);
        ECS.addComponent(platform, Rigidbody{true});
        ECS.addComponent(platform, Material());
        auto model = Model("platform_debug");
        model.color = {0.05, 0.15, 0.05, 1};
        ECS.addComponent(platform, model);
    }

    ECS.getSystem<PhysicsSystem>()->gravity = {0, 20000.f};
    ECS.getSystem<PhysicsSystem>()->substep_count = 16U;
    this->setPhysicsTickrate(120.f);
    
}

void Demo::setupAnimationForProtagonist(Entity entity) {
    auto def_size = vec2f(300.f, 300.f);
    auto offset = vec2f(10.f, -def_size.y / 3.5f);
    MovingSprite idle_moving;
    {
        Sprite idle_sprite = Sprite(Texture("idle"), def_size);
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
            Sprite jumping_spr = Sprite(Texture("attack"), def_size);
            jumping_spr.centered = true;
            jumping_spr.hframes = 4;
            jumping_spr.vframes = 1;
            build.addNode("attack", MovingSprite::allFrames(jumping_spr, 0.30f, false));
        }
        {
            Sprite jumping_spr = Sprite(Texture("roll"), def_size);
            jumping_spr.centered = true;
            jumping_spr.hframes = 12;
            jumping_spr.vframes = 1;
            build.addNode("roll", MovingSprite::allFrames(jumping_spr, 0.30f, false));
        }
        {
            Sprite running_sprite = Sprite(Texture("running"), def_size);
            running_sprite.centered = true;
            running_sprite.hframes = 10;
            running_sprite.vframes = 1;
            build.addNode("run",  MovingSprite::allFrames(running_sprite, 0.6f));
        }
        {
            Sprite jumping_spr = Sprite(Texture("jump-up"), def_size);
            jumping_spr.centered = true;
            jumping_spr.hframes = 3;
            jumping_spr.vframes = 1;
            build.addNode("jump", MovingSprite::allFrames(jumping_spr, 0.25f, false));
        }
        {
            Sprite jumpfall_spr = Sprite(Texture("jumpfall"), def_size);
            jumpfall_spr.centered = true;
            jumpfall_spr.hframes = 2;
            jumpfall_spr.vframes = 1;
            build.addNode("jumpfall", MovingSprite::allFrames(jumpfall_spr, 0.25f, false));
        }
        {
            Sprite falling_spr = Sprite(Texture("jump-down"), def_size);
            falling_spr.centered = true;
            falling_spr.hframes = 3;
            falling_spr.vframes = 1;
            build.addNode("fall", MovingSprite::allFrames(falling_spr, 0.25f, false));
        }

        build.addEdge("idle", "run", [this](Entity owner) {
            return abs(ECS.getComponent<Rigidbody>(owner)->velocity.x) >
                   25.f;
        });
        build.addEdge("run", "idle", [this](Entity owner, bool isended) {
            return abs(ECS.getComponent<Rigidbody>(owner)->velocity.x) <
                   25.f;
        });
        auto isJumping = [this](Entity owner) {
            return ECS.getComponent<Rigidbody>(owner)->velocity.y <
                   -100.f;
        };
        auto isFalling = [this](Entity owner) {
            return ECS.getComponent<Rigidbody>(owner)->velocity.y >
                   25.f;
        };
        auto hasFallen = [&](Entity owner) {
            return isProtagonistGrounded != false;
        };
        auto isAttacking = [&](Entity owner) {
            return isProtagonistAttacking;
        };
        auto isRolling = [&](Entity owner) {
            return isProtagonistRolling;
        };
        build.addEdge("idle", "attack", isAttacking);
        build.addEdge("run", "attack", isAttacking);
        build.addEdge("attack", "idle");

        build.addEdge("run", "roll", isRolling);
        build.addEdge("roll", "run");

        build.addEdge("idle", "fall", isFalling);
        build.addEdge("run", "fall", isFalling);
        build.addEdge("idle", "jump", isJumping);
        build.addEdge("run", "jump", isJumping);
        build.addEdge("jump", "jumpfall", isFalling);
        build.addEdge("jumpfall", "fall");
        build.addEdge("fall", "idle", hasFallen);
    }
    {
        std::array<Vertex, 4U> verts;
        for(int i = 0; i < verts.size(); i++) {
            verts[i].color = {1, 0, 0};
            verts[i].uv = {0, 0};

            auto v = protagonist_shape[i];
            verts[i].position = vec3f(v.x, v.y, 0.f);
        }
    }
    auto anim_sprite = AnimatedSprite(build);
    anim_sprite.position_offset = offset;
    ECS.addComponent(entity, anim_sprite);
}
void Demo::onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller) {
}

void Demo::onRender(Device&, const FrameInfo& frame) {
}
void Demo::onUpdate(const float delta_time, Window& window, KeyboardController& controller) 
{
    auto& phy_sys = *ECS.getSystem<PhysicsSystem>();
    ECS.getComponent<Transform>(mouse_entity)->position =
        controller.global_mouse_pos();
    if(ECS.isEntityAlive(protagonist)){
        isProtagonistGroundedSec += delta_time;
        if(isProtagonistGrounded) {
            isProtagonistGroundedSec = 0.f;
        } 
        if(ECS.isEntityAlive(protagonist)){
            auto direction = controller.movementInPlane2D().x;
            ECS.getComponent<Rigidbody>(protagonist)->velocity.x +=
                    direction * 
                    (protagonist_speed * 0.5 + protagonist_speed / 2.f *
                            (isProtagonistGrounded != false)) *
                    delta_time;
            auto& rb = *ECS.getComponent<Rigidbody>(protagonist);
            if (controller.get(eKeyMappings::Jump).pressed && isProtagonistGroundedSec < cayote_time) {
                isProtagonistGroundedSec = cayote_time;
                isProtagonistGrounded = false;
                vec2f direction = vec2f(0, -10);
                direction.x = controller.movementInPlane2D().x;
                direction = normal(direction);
                rb.velocity += 500.f * direction;
            }
        }
        auto& rb = *ECS.getComponent<Rigidbody>(protagonist);
        auto& spr = *ECS.getComponent<AnimatedSprite>(protagonist);
        bool isRunningLeft = rb.velocity.x < 0.f;
        if(abs(rb.velocity.x) > 10.f) {
            if(spr.flipX ^ isRunningLeft) {
                spr.position_offset.x *= -1.f;
            }
            spr.flipX = isRunningLeft ?  true : false;
        }
    }

    static vec2f last_pos;
    if (controller.get(eKeyMappings::Ability1).pressed) {
        last_pos = vec2f(controller.global_mouse_pos());
        Sprite spr = Sprite(crate_texture, {cube_side_len, cube_side_len});
        spr.order = 0;
        Rigidbody rb;
        auto col = Collider(cube_model_shape); col.collider_layer = ITEM;
        auto entity = ECS.createEntity();
        gui_manager.alias(entity, "cube");

        last_created_crate = entity;
        ECS.addComponent(
                entity, Transform(vec2f(controller.global_mouse_pos()), 0.f)
        );
        ECS.addComponent(entity, col);
        ECS.addComponent(entity, rb);
        Material mat; mat.static_friction = 0.8f;
        ECS.addComponent(entity, mat);
        ECS.addComponent(entity, spr);
    }
    if (controller.get(eKeyMappings::Ability2).pressed) {
        Sprite spr = Sprite(Texture("dummy"), {cube_side_len * 2, cube_side_len * 2});
        Rigidbody rb;
        auto shape = cube_model_shape;
        for(auto& v : shape) v.y *= 2.f;
        auto col = Collider(shape); col.collider_layer = FRIENDLY;
        auto entity = ECS.createEntity();
        gui_manager.alias(entity, "dummy");

        ECS.addComponent(
                entity, Transform(vec2f(controller.global_mouse_pos()), 0.f)
        );
        ECS.addComponent(entity, col);
        ECS.addComponent(entity, rb);
        Material mat; mat.static_friction = 0.8f;
        ECS.addComponent(entity, mat);
        ECS.addComponent(entity, spr);
    }
    if (controller.get(eKeyMappings::Ability3).pressed) {
        if(isProtagonistGrounded && abs(ECS.getComponent<Rigidbody>(protagonist)->velocity.x) < 50.f) {
            isProtagonistAttacking = true;
        }
    }else {
            isProtagonistAttacking = false;
    }
    if (controller.get(eKeyMappings::Ability4).pressed) {
        if(isProtagonistGrounded && abs(ECS.getComponent<Rigidbody>(protagonist)->velocity.x) > 25.f) {
            auto vel = ECS.getComponent<Rigidbody>(protagonist)->velocity.x;
            auto rolling_vel = 500.f;
            ECS.getComponent<Rigidbody>(protagonist)->velocity.x += copysign(rolling_vel - abs(vel), vel);

            isProtagonistRolling = true;
            timerSinceRoll.restart();
        }
    }else {
        if(timerSinceRoll.getElapsedTime() > 0.5f && timerSinceRoll.getElapsedTime() < 1.f) {
            ECS.getComponent<Rigidbody>(protagonist)->velocity.x *= 0.95f;
        }
        isProtagonistRolling = false;
    }
    
    auto mouse_pos = controller.global_mouse_pos();
    if (controller.get(eKeyMappings::Shoot).pressed) {
        auto entities= ECS.getSystem<MouseSelectionSystem>()->query(mouse_pos);
        if(entities.size() != 0) {
            if(ECS.isEntityAlive(mouse_entity)) {
                ECS.removeComponentIfExists<Constraint>(mouse_entity);
            }else {
                mouse_entity = ECS.createEntity();
            }

            auto target = entities.front();
            auto mouse_transform = ECS.getComponent<Transform>(mouse_entity);
            auto target_transform = ECS.getComponent<Transform>(target);
            auto mouse_obj_constr = Constraint::Builder()
                .setCompliance(0.1e-7f)
                .enableCollision()
                .setHinge(mouse_pos)
                .addAnchorEntity(mouse_entity, *mouse_transform)
                .addConstrainedEntity(target, *target_transform)
                .build();
                
            ECS.addComponent(mouse_entity, mouse_obj_constr);
            EMP_LOG(INFO) << "anchor added";
        }
    }
    if(controller.get(eKeyMappings::Shoot).released) {

        auto entities= ECS.getSystem<MouseSelectionSystem>()->query(mouse_pos);

        if(entities.size() == 2 && ECS.getComponent<Constraint>(entities.front()) == nullptr) {
            assert(ECS.getComponent<Transform>(entities.front()) && ECS.getComponent<Transform>(entities.back()));
            assert(ECS.getComponent<Collider>(entities.front()) && ECS.getComponent<Collider>(entities.back()));
            assert(ECS.getComponent<Rigidbody>(entities.front()) && ECS.getComponent<Rigidbody>(entities.back()));

            auto chain = Constraint::Builder()
                .setCompliance(0.1e-7f)
                .enableCollision()
                .setFixed()
                .addConstrainedEntity(entities.front(), *ECS.getComponent<Transform>(entities.front()))
                .addConstrainedEntity(entities.back(), *ECS.getComponent<Transform>(entities.back()))
                .build();
            ECS.getComponent<Collider>(entities.front())->collider_layer = FRIENDLY;
            ECS.getComponent<Collider>(entities.back())->collider_layer = FRIENDLY;
            EMP_LOG(INFO) << "constraint added";
            ECS.addComponent(entities.front(), chain);

        }
        ECS.removeComponentIfExists<Constraint>(mouse_entity);
    }

    {
        static vec2f last_mouse_pos;
        auto emitter = ECS.getComponent<ParticleEmitter>(mouse_entity);
        assert(emitter);
        emitter->setLine(last_mouse_pos - mouse_pos);
        last_mouse_pos = mouse_pos;
    }

    if(controller.get(eKeyMappings::Shoot).released) {
        auto emitter = ECS.getComponent<ParticleEmitter>(mouse_entity);
        assert(emitter);
        emitter->enabled = false;
    }
    if (controller.get(eKeyMappings::Shoot).pressed) {
        auto emitter = ECS.getComponent<ParticleEmitter>(mouse_entity);
        assert(emitter);
        emitter->enabled = true;
    }
}
int main()
{
    {
        Demo demo;
        demo
            .run();
    }
}


