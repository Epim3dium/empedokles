#include "graphics/animated_sprite.hpp"
#include "gui/gui_manager.hpp"
#include "io/keyboard_controller.hpp"
#include "physics/rigidbody.hpp"
#include "scene/app.hpp"
#include <vector>
using namespace emp;

class MouseSelectionSystem : public System<Transform, Collider, Rigidbody> {
public:
    std::vector<Entity> query(vec2f point) {
        std::vector<Entity> result;
        for(auto e : entities) {
            auto& shape = getComponent<Collider>(e);
            auto& transform = getComponent<Transform>(e);
            auto transformed_outline = shape.transformed_outline();
            
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
    ITEM
};
class Demo : public App {
    public:

        Texture crate_texture;
        Entity mouse_entity;
        Entity protagonist;

        
        float protagonist_speed = 600.f;
        Entity isProtagonistGrounded;
        float isProtagonistGroundedSec;
        static constexpr float cayote_time = 0.25f;
        static constexpr float cube_side_len = 75.f;
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


        void setupAnimationForProtagonist();
        void onRender(Device&, const FrameInfo& frame) override final;
        void onSetup(Window& window, Device& device) override final;
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final;
        void onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller)override final;

        Demo()
            : App(1440, 810, {{"../assets/models/colored_cube.obj", "cube"}},
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
    gui_manager.alias(ECS.world(), "world_entity");
    ECS.registerSystem<MouseSelectionSystem>();

    crate_texture = Texture("crate");
    controller.bind(eKeyMappings::Shoot, GLFW_MOUSE_BUTTON_LEFT);

    controller.bind(eKeyMappings::Ability1, GLFW_KEY_C);
    controller.bind(eKeyMappings::Ability2, GLFW_KEY_T);
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

    {
        Rigidbody rb;
        rb.useAutomaticMass = true;
        rb.isRotationLocked = true;
        ECS.addComponent(protagonist, Transform(vec2f(), 0.f));
        std::vector<vec2f> protagonist_shape = {
                vec2f(-100.f/4, -100.f/1.5),
                vec2f(-100.f/4, 100.f/1.5),
                vec2f(100.f/4, 100.f/1.5),
                vec2f(100.f/4, -100.f/1.5)
        };
        auto col = Collider(protagonist_shape);
        col.collider_layer = PLAYER;

        ECS.addComponent(protagonist, col);
        ECS.addComponent(protagonist, rb);

        auto mat = Material(); 
        ECS.addComponent(protagonist, mat);

        setupAnimationForProtagonist();
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
    for (auto o : ops) {
        auto platform = ECS.createEntity();
        gui_manager.alias(platform, "platform");
        ECS.addComponent(
                platform, Transform(o.first, o.second, {getWidth() / cube_side_len, 1.f})
        );
        auto db_shape = DebugShape(device, cube_model_shape);
        db_shape.fill_color *= 0.5f;
        ECS.addComponent(platform, db_shape);
        auto col = Collider(cube_model_shape);
        col.collider_layer = GROUND;
        ECS.addComponent(platform, col);
        ECS.addComponent(platform, Rigidbody{true});
        ECS.addComponent(platform, Material());
    }
    // for (auto& marker : markers) {
    //     marker = coordinator.createEntity();
    //     coordinator.addComponent(marker, Transform(vec2f(INFINITY, INFINITY), 0.f, {0.1f, 0.1f}));
    //     coordinator.addComponent(marker, DebugShape(device, unit_cube, glm::vec4(0.8, 0.8, 1, 1)));
    // }

    ECS.getSystem<PhysicsSystem>()->gravity = 1000.f;
    ECS.getSystem<PhysicsSystem>()->substep_count = 16U;
    this->setPhysicsTickrate(60.f);
    
}

void Demo::setupAnimationForProtagonist() {
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
        build.addEdge("idle", "fall", isFalling);
        build.addEdge("run", "fall", isFalling);
        build.addEdge("idle", "jump", isJumping);
        build.addEdge("run", "jump", isJumping);
        build.addEdge("jump", "jumpfall", isFalling);
        build.addEdge("jumpfall", "fall");
        build.addEdge("fall", "idle", hasFallen);
    }
    auto anim_sprite = AnimatedSprite(build);
    anim_sprite.position_offset = offset;
    ECS.addComponent(protagonist, anim_sprite);
}
void Demo::onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller) {
}

void Demo::onRender(Device&, const FrameInfo& frame) {
}
void Demo::onUpdate(const float delta_time, Window& window, KeyboardController& controller) 
{
    auto& phy_sys = *ECS.getSystem<PhysicsSystem>();
    for(auto e : phy_sys.getEntities()) {
        if(!ECS.getComponent<DebugShape>(e)) {
            continue;
        }
    }
    int i = 0;
    {
        ECS.getComponent<Transform>(mouse_entity)->position =
                controller.global_mouse_pos();
    }
    if(ECS.isEntityAlive(protagonist)){
        isProtagonistGroundedSec += delta_time;
        if(isProtagonistGrounded) {
            isProtagonistGroundedSec = 0.f;
        } 
        if(ECS.isEntityAlive(protagonist)){
            ECS.getComponent<Rigidbody>(protagonist)->velocity.x +=
                    controller.movementInPlane2D().x *
                    (protagonist_speed / 2.f + protagonist_speed / 2.f *
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
        Rigidbody rb; rb.useAutomaticMass = true;
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
    
    if (controller.get(eKeyMappings::Shoot).pressed) {
        auto mouse_pos = controller.global_mouse_pos();
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
                .setConnectionGlobalPoint(mouse_pos)
                .addAnchorEntity(mouse_entity, *mouse_transform)
                .addConstrainedEntity(target, *target_transform)
                .build();
                
            ECS.addComponent(mouse_entity, mouse_obj_constr);
            EMP_LOG(INFO) << "constraint added";
        }
    }
    if(controller.get(eKeyMappings::Shoot).released) {
        ECS.removeComponentIfExists<Constraint>(mouse_entity);
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


