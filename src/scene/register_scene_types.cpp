#include "register_scene_types.hpp"
#include "core/coordinator.hpp"
#include "graphics/debug_shape.hpp"
#include "graphics/debug_shape_system.hpp"
#include "io/keyboard_controller.hpp"
#include "physics/collider.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
#include "physics/rigidbody.hpp"

namespace emp {
    void registerSceneTypes() {
        coordinator.registerComponent<KeyboardController>();
        coordinator.registerComponent<Transform>();

        coordinator.registerComponent<Material>();
        coordinator.registerComponent<Collider>();
        coordinator.registerComponent<Rigidbody>();

        coordinator.registerComponent<Model>();
        coordinator.registerComponent<Texture>();
        coordinator.registerComponent<DebugShape>();
    }
    void registerSceneSystems(Device& device) {
        coordinator.registerSystem<KeyboardControllerSystem>();

        coordinator.registerSystem<TransformSystem>();
        coordinator.registerSystem<RigidbodySystem>();
        coordinator.registerSystem<ColliderSystem>();
        coordinator.registerSystem<PhysicsSystem>();

        coordinator.registerSystem<DebugShapeSystem>(std::ref(device));
        coordinator.registerSystem<TexturedModelsSystem>(std::ref(device));
    }
}
