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
template<class ...T>
void registerComponents(TypePack<T...> pack) {
    (ECS.registerComponent<T>(), ...);
}
void registerSceneTypes() {
    registerComponents(AllComponentTypes());
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
