#include "register_scene_types.hpp"
#include "core/coordinator.hpp"
#include "graphics/animated_sprite_system.hpp"
#include "graphics/debug_shape.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/sprite.hpp"
#include "graphics/animated_sprite.hpp"
#include "graphics/sprite_system.hpp"
#include "io/keyboard_controller.hpp"
#include "physics/collider.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
#include "physics/rigidbody.hpp"
#include "scene/behaviour.hpp"

namespace emp {
void registerSceneTypes() {
    ECS.registerComponent<Transform>();
    ECS.registerComponent<Behaviour>();

    ECS.registerComponent<Constraint>();
    ECS.registerComponent<Material>();
    ECS.registerComponent<Collider>();
    ECS.registerComponent<Rigidbody>();

    ECS.registerComponent<Model>();
    ECS.registerComponent<Texture>();
    ECS.registerComponent<DebugShape>();

    ECS.registerComponent<Sprite>();
    ECS.registerComponent<AnimatedSprite>();
}
void registerSceneSystems(Device& device) {
    ECS.registerSystem<TransformSystem>();
    ECS.registerSystem<BehaviourSystem>();

    ECS.registerSystem<RigidbodySystem>();
    ECS.registerSystem<ColliderSystem>();
    ECS.registerSystem<ConstraintSystem>();
    ECS.registerSystem<PhysicsSystem>();

    ECS.registerSystem<SpriteSystem>(std::ref(device));
    ECS.registerSystem<AnimatedSpriteSystem>(std::ref(device));
    ECS.registerSystem<DebugShapeSystem>(std::ref(device));
    ECS.registerSystem<TexturedModelsSystem>(std::ref(device));
    ECS.addComponent(ECS.world(), Transform(vec2f(0, 0), 0.f, {1.f, 1.f}));
}
} // namespace emp
