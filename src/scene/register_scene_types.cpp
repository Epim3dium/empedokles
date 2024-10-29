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
    coordinator.registerComponent<Transform>();
    coordinator.addComponent(coordinator.world(), Transform(vec2f(0, 0), 0.f, {1.f, 1.f}));
    coordinator.registerComponent<Behaviour>();

    coordinator.registerComponent<Constraint>();
    coordinator.registerComponent<Material>();
    coordinator.registerComponent<Collider>();
    coordinator.registerComponent<Rigidbody>();

    coordinator.registerComponent<Model>();
    coordinator.registerComponent<Texture>();
    coordinator.registerComponent<DebugShape>();

    coordinator.registerComponent<Sprite>();
    coordinator.registerComponent<AnimatedSprite>();
}
void registerSceneSystems(Device& device) {
    coordinator.registerSystem<TransformSystem>();
    coordinator.registerSystem<BehaviourSystem>();

    coordinator.registerSystem<RigidbodySystem>();
    coordinator.registerSystem<ColliderSystem>();
    coordinator.registerSystem<ConstraintSystem>();
    coordinator.registerSystem<PhysicsSystem>();

    coordinator.registerSystem<SpriteSystem>(std::ref(device));
    coordinator.registerSystem<AnimatedSpriteSystem>(std::ref(device));
    coordinator.registerSystem<DebugShapeSystem>(std::ref(device));
    coordinator.registerSystem<TexturedModelsSystem>(std::ref(device));
    coordinator.addComponent(coordinator.world(), Transform(vec2f(0, 0), 0.f, {1.f, 1.f}));
}
} // namespace emp
