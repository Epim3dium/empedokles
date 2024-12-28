#ifndef EMP_SERIALIZED_COMPONENTS_HPP
#define EMP_SERIALIZED_COMPONENTS_HPP
#include "graphics/animated_sprite.hpp"
#include "graphics/sprite.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/material.hpp"
#include "scene/transform.hpp"
#include "io/serializer.hpp"
namespace emp {
template<>
struct SerialConvert<Transform> {
    void encode(const Transform& var, IGlobWriter& writer);
    void decode(Transform& var, IGlobReader& reader);
};
template<>
struct SerialConvert<Constraint> {
    void encode(const Constraint& var, IGlobWriter& writer);
    void decode(Constraint& var, IGlobReader& reader);
};
template<>
struct SerialConvert<Material> {
    void encode(const Material& var, IGlobWriter& writer);
    void decode(Material& var, IGlobReader& reader);
};
template<>
struct SerialConvert<Collider> {
    void encode(const Collider& var, IGlobWriter& writer);
    void decode(Collider& var, IGlobReader& reader);
};
template<>
struct SerialConvert<Rigidbody> {
    void encode(const Rigidbody& var, IGlobWriter& writer);
    void decode(Rigidbody& var, IGlobReader& reader);
};
template<>
struct SerialConvert<Sprite> {
    void encode(const Sprite& var, IGlobWriter& writer);
    void decode(Sprite& var, IGlobReader& reader);
};
template<>
struct SerialConvert<AnimatedSprite> {
    void encode(const AnimatedSprite& var, IGlobWriter& writer);
    void decode(AnimatedSprite& var, IGlobReader& reader);
};
// template<>
// struct SerialConvert<Model> {
//     void encode(const Model& var, IGlobWriter& writer);
//     void decode(Model& var, IGlobReader& reader);
// };

}
#endif
