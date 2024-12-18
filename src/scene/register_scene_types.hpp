#ifndef EMP_REGISTER_SCENE_TYPES_HPP
#define EMP_REGISTER_SCENE_TYPES_HPP
#include "graphics/animated_sprite.hpp"
#include "graphics/animated_sprite_system.hpp"
#include "graphics/debug_shape.hpp"
#include "graphics/debug_shape_system.hpp"
#include "graphics/model.hpp"
#include "graphics/sprite_system.hpp"
#include "graphics/texture.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/material.hpp"
#include "physics/physics_system.hpp"
#include "scene/behaviour.hpp"
#include "scene/transform.hpp"
#include "templates/type_pack.hpp"
namespace emp {
    struct Device;

    typedef TypePack<Transform,
        Behaviour,
        Constraint,
        Material,
        Collider,
        Rigidbody,
        Model,
        DebugShape,
        Sprite,
        AnimatedSprite> AllComponentTypes;

    void registerSceneTypes();
    void registerSceneSystems(Device& device);
};
#endif
