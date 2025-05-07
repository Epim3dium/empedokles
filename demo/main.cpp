#include "core/coordinator.hpp"
#include "debug/log.hpp"
#include "graphics/animated_sprite.hpp"
#include "graphics/particle_system.hpp"
#include "gui/gui_manager.hpp"
#include "io/keyboard_controller.hpp"
#include "math/math_func.hpp"
#include "physics/collider.hpp"
#include "utils/perlin.hpp"
#include "physics/rigidbody.hpp"
#include "scene/app.hpp"
#include <cstdio>
#include <vector>
using namespace emp;

std::vector<vec2f> unit_cube = {
        vec2f(-1.f/2.f, -1.f/2.f),
        vec2f(-1.f/2.f, 1.f/2.f),
        vec2f(1.f/2.f, 1.f/2.f),
        vec2f(1.f/2.f, -1.f/2.f)
};
struct GroundSystem {
    std::vector<Entity> blocks;
    float freq = 1.f;
    float amp = 400.f;
    vec2f base_pos;
    uint32_t seed = 0;
    uint32_t cur_block_idx = 0;

    float block_len;
    float amp_inc = 4;

    void setBlock(Coordinator& ECS, uint32_t idx) {
        auto spacing = block_len;
        auto block = blocks[idx%blocks.size()];
        auto h = Perlin::perlin((float)seed / blocks.size(), (float)seed / blocks.size() * 0.49) * amp; 
        auto next_h = Perlin::perlin((float)(seed+1) / blocks.size(), (float)(seed+1) / blocks.size() * 0.49) * (amp + amp_inc);
        vec2f diff (spacing, next_h-h);
        float diff_l = length(diff);
        auto ang = angle(vec2f(1, 0), diff);
        auto& transform = *ECS.getComponent<Transform>(block);

        vec2f diff_perp = {-diff.y, diff.x};
        vec2f pos = vec2f{spacing * (idx + 0.5), (h+next_h)/2.f} + base_pos;
        float height = block_len * 9;
        pos += normal(diff_perp) * (height/2);
        transform = Transform(pos, ang, {diff_l, height});
        amp += amp_inc;
    }

    void setCurBlock(Coordinator& ECS) {
        seed ++;
        setBlock(ECS, cur_block_idx);
        cur_block_idx++;
    }
    void init(Coordinator& ECS, Device& device, vec2f player_pos, uint32_t nb_blocks = 32, float seg_len = 70.f) {
        block_len = seg_len;
        base_pos = {player_pos.x - block_len * 5, player_pos.y + 100.f + block_len * 2};
        seed = player_pos.x / block_len + player_pos.y / block_len * 3.f + time(NULL) % nb_blocks;

        auto col_shape = unit_cube;
        auto builder = ModelAsset::Builder();
        for(auto v : unit_cube) {
            builder.vertices.push_back({});
            builder.vertices.back().position = vec3f(v.x, v.y, 0);
        }
        builder.vertices[1].position.x *= 3;
        builder.vertices[2].position.x *= 3;
        builder.indices = {0, 1, 2, 2, 3, 0};
        Model::create("ground", device, builder);

        auto model = Model("ground");
        auto rb = Rigidbody(true);
        auto mat = Material();
        auto col = Collider(col_shape);
        for(int i = 0; i < nb_blocks; i++) {
            auto block = ECS.createEntity();
            blocks.push_back(block);
            ECS.addComponent(block, Transform(base_pos));
            ECS.addComponent(block, model);
            ECS.addComponent(block, rb);
            ECS.addComponent(block, mat);
            ECS.addComponent(block, col);
        }
        for(int i = 0; i < nb_blocks; i++) {
            setCurBlock(ECS);
        }

    }
    void update(Coordinator& ECS, vec2f player_position) {
        const float threshold = block_len * blocks.size()/2;
        auto last = cur_block_idx + blocks.size();
        auto block = blocks[last % blocks.size()];
        auto trans = ECS.getComponent<Transform>(block);
        while(trans && player_position.x - trans->position.x  > threshold) {
            setCurBlock(ECS);
        }
    }
};
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

struct Player {
    float wheel_size = 17.f;
    std::set<Entity> driver;
    std::set<Entity> wheels;
    std::set<Entity> chassis;
    std::set<Entity> constraints;
};
enum CollisionLayers {
    GROUND,
    PLAYER,
};
class Demo : public App {
    public:

        Texture crate_texture;
        Entity mouse_entity;
        static constexpr float cube_side_len = 70.f;
        std::vector<vec2f> cube_model_shape = {
                vec2f(-cube_side_len/2, -cube_side_len/2),
                vec2f(-cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, cube_side_len/2),
                vec2f(cube_side_len/2, -cube_side_len/2)
        };
        Entity last_created_crate = 0;
        Player player;
        GroundSystem ground;


        void onRender(Device&, const FrameInfo& frame) override final;
        void setupPlayer(Player&);
        void onSetup(Window& window, Device& device) override final;
        void onUpdate(const float delta_time, Window& window, KeyboardController& controller) override final;
        void onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller)override final;

        Demo(int w = 1440, int h = 810)
            : App(w, h, {{"../assets/models/colored_cube.obj", "cube"}},
                {
                    {"../assets/textures/dummy.png", "dummy"},
                    {"../assets/textures/crate.jpg", "crate"},
                    {"../assets/textures/HCR/Car.png", "car"},
                    {"../assets/textures/HCR/Tire.png", "tire"},
                })
                {}
};
void Demo::onSetup(Window& window, Device& device) {
    gui_manager.alias(ECS.world(), "world_entity");
    ECS.registerSystem<MouseSelectionSystem>();
    ECS.getSystem<ColliderSystem>()->disableCollision(PLAYER, PLAYER);

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

    ECS.getSystem<PhysicsSystem>()->gravity = {0, 20000.f};
    ECS.getSystem<PhysicsSystem>()->substep_count = 16U;
    this->setPhysicsTickrate(120.f);

    
    setupPlayer(player);
    ground.init(ECS, device, {-500, 50});
}
void Demo::setupPlayer(Player& player) {
    std::vector<vec2f> wheel_shape;
    static const int wheel_vert_count = 32U;
    for(int i = 0; i < wheel_vert_count; i++) {
        float t = static_cast<float>(i) / static_cast<float>( wheel_vert_count );
        wheel_shape.push_back({cosf(t * EMP_PI * 2.f) * player.wheel_size, -sinf(t * EMP_PI * 2.f) * player.wheel_size});
    }
    {
        auto builder = ModelAsset::Builder();
        builder.vertices.push_back({{0, 0, 0}});
        for(auto v : wheel_shape) {
            builder.vertices.push_back({});
            builder.vertices.back().position = vec3f(v.x, v.y, 0);
        }
        for(int i = 1; i < wheel_vert_count+1; i++) {
            builder.indices.push_back(0);
            builder.indices.push_back(i);
            if(i + 1 == wheel_vert_count+1) {
                //change here to make uniform wheel 0->1
                builder.indices.push_back(0);
            }else {
                builder.indices.push_back(i + 1);
            }
        }
        Model::create("wheel", device, builder);
    }
    std::vector<vec2f> wheel_positions = {{50, player.wheel_size * 1.5}, {-50, player.wheel_size * 1.5}};
    auto col = Collider(wheel_shape); col.collider_layer = PLAYER;
    auto rb = Rigidbody(false);
    auto mat = Material{}; mat.dynamic_friction = 0.8f;
    auto model = Model("wheel"); model.color = {0.3, 0.3, 0.3, 1};
    auto spr = Sprite(Texture("tire"), {player.wheel_size*2, player.wheel_size*2});
    for (auto wheel_pos : wheel_positions) {
        auto wheel = ECS.createEntity();
        ECS.addComponent(wheel, spr);
        ECS.addComponent(wheel, col);
        ECS.addComponent(wheel, rb);
        ECS.addComponent(wheel, mat);
        ECS.addComponent( wheel, Transform(wheel_pos, 0.f, {1.f, 1.f}));
        gui_manager.alias(wheel, "wheel");
        player.wheels.insert(wheel);
    }

    auto chassis = ECS.createEntity();
    player.chassis.insert(chassis);
    col = Collider(cube_model_shape);
    col.collider_layer = PLAYER;
    auto sprite = Sprite(Texture("car"), {70.f, 90.f});
    ECS.addComponent(chassis, col);
    ECS.addComponent(chassis, Rigidbody(false));
    ECS.addComponent(chassis, Material());
    ECS.addComponent(chassis, sprite);
    ECS.addComponent(chassis, Transform({0, -player.wheel_size/3}, 0.f, {2.25f, 0.75}));
    gui_manager.alias(chassis, "chassis");

    for(auto wheel : player.wheels) {
        auto constr_entity = ECS.createEntity();
        Constraint::Builder builder;
        builder
            .addConstrainedEntity(wheel, *ECS.getComponent<Transform>(wheel))
            .addConstrainedEntity(chassis, *ECS.getComponent<Transform>(chassis))
            .enableCollision(false)
            .setCompliance(2e-7f)
            .setHinge(ECS.getComponent<Transform>(wheel)->position);
        ECS.addComponent<Constraint>(constr_entity, builder.build());
        gui_manager.alias(constr_entity, "spring");
    }
}

// void Demo::setupAnimationForProtagonist(Entity entity) {
//     auto def_size = vec2f(300.f, 300.f);
//     auto offset = vec2f(10.f, -def_size.y / 3.5f);
//     MovingSprite idle_moving;
//     {
//         Sprite idle_sprite = Sprite(Texture("idle"), def_size);
//         idle_sprite.centered = true;
//         idle_sprite.hframes = 10;
//         idle_sprite.vframes = 1;
//         idle_moving.sprite = idle_sprite;
//         for(int i = 0; i < idle_sprite.frameCount(); i++) {
//             idle_moving.add(i, 0.085f);
//         }
//     }
//     AnimatedSprite::Builder build("idle", idle_moving);
//     {
//         {
//             Sprite running_sprite = Sprite(Texture("running"), def_size);
//             running_sprite.centered = true;
//             running_sprite.hframes = 10;
//             running_sprite.vframes = 1;
//             build.addNode("run",  MovingSprite::allFrames(running_sprite, 0.6f));
//         }
//         {
//             Sprite jumping_spr = Sprite(Texture("jump-up"), def_size);
//             jumping_spr.centered = true;
//             jumping_spr.hframes = 3;
//             jumping_spr.vframes = 1;
//             build.addNode("jump", MovingSprite::allFrames(jumping_spr, 0.25f, false));
//         }
//         {
//             Sprite jumpfall_spr = Sprite(Texture("jumpfall"), def_size);
//             jumpfall_spr.centered = true;
//             jumpfall_spr.hframes = 2;
//             jumpfall_spr.vframes = 1;
//             build.addNode("jumpfall", MovingSprite::allFrames(jumpfall_spr, 0.25f, false));
//         }
//         {
//             Sprite falling_spr = Sprite(Texture("jump-down"), def_size);
//             falling_spr.centered = true;
//             falling_spr.hframes = 3;
//             falling_spr.vframes = 1;
//             build.addNode("fall", MovingSprite::allFrames(falling_spr, 0.25f, false));
//         }
//
//         build.addEdge("idle", "run", [this](Entity owner) {
//             return abs(ECS.getComponent<Rigidbody>(owner)->velocity.x) >
//                    25.f;
//         });
//         build.addEdge("run", "idle", [this](Entity owner, bool isended) {
//             return abs(ECS.getComponent<Rigidbody>(owner)->velocity.x) <
//                    25.f;
//         });
//         auto isJumping = [this](Entity owner) {
//             return ECS.getComponent<Rigidbody>(owner)->velocity.y <
//                    -100.f;
//         };
//         auto isFalling = [this](Entity owner) {
//             return ECS.getComponent<Rigidbody>(owner)->velocity.y >
//                    25.f;
//         };
//         build.addEdge("idle", "fall", isFalling);
//         build.addEdge("run", "fall", isFalling);
//         build.addEdge("idle", "jump", isJumping);
//         build.addEdge("run", "jump", isJumping);
//         build.addEdge("jump", "jumpfall", isFalling);
//         build.addEdge("jumpfall", "fall");
//     }
//     auto anim_sprite = AnimatedSprite(build);
//     anim_sprite.position_offset = offset;
//     ECS.addComponent(entity, anim_sprite);
// }
void Demo::onFixedUpdate(const float delta_time, Window& window, KeyboardController& controller) {
}

void Demo::onRender(Device&, const FrameInfo& frame) {
}
void Demo::onUpdate(const float delta_time, Window& window, KeyboardController& controller) 
{
    auto& phy_sys = *ECS.getSystem<PhysicsSystem>();
    ECS.getComponent<Transform>(mouse_entity)->position =
        controller.global_mouse_pos();

    static vec2f last_pos;
    if (controller.get(eKeyMappings::Ability1).held) {
        last_pos = vec2f(controller.global_mouse_pos());
        Sprite spr = Sprite(crate_texture, {cube_side_len, cube_side_len});
        Rigidbody rb;
        auto col = Collider(cube_model_shape); 
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
            ECS.getComponent<Collider>(entities.front())->collider_layer = PLAYER;
            ECS.getComponent<Collider>(entities.back())->collider_layer = PLAYER;
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
    static const float drive_speed = 500000.f;
    static const float max_velocity = 3000.f;
    float direction = 0.f;
    if(controller.get(eKeyMappings::MoveRight).held) {
        direction = 1.f;
    }
    if(controller.get(eKeyMappings::MoveLeft).held) {
        direction = -1.f;
    }
    for(auto wheel : player.wheels) {
        auto rb = ECS.getComponent<Rigidbody>(wheel);
        if(!rb) continue;
        rb->torque += drive_speed * rb->inertia() * delta_time * direction;
    }
    const float camera_damping = 10.0f;
    auto& viewer_transform =
            *ECS.getComponent<Transform>(viewer_entity);
    auto player_pos = ECS.getComponent<Transform>(*player.chassis.begin())->position;
    viewer_transform.position = viewer_transform.position * (1-camera_damping*delta_time) +
         player_pos * (camera_damping * delta_time);

    ground.update(ECS, player_pos);
}
int main()
{
    {
        Demo demo;
        demo
            .run();
    }
}


