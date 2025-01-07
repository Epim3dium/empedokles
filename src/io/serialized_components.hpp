#ifndef EMP_SERIALIZED_COMPONENTS_HPP
#define EMP_SERIALIZED_COMPONENTS_HPP
#include "graphics/animated_sprite.hpp"
#include "graphics/model.hpp"
#include "graphics/sprite.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/material.hpp"
#include "scene/transform.hpp"
#include "io/serial_convert.hpp"
#include "serialized_containers.hpp"
namespace emp {
template<>
struct SerialConvert<Transform> {
    void encode(const Transform& var, IBlobWriter& writer);
    void decode(Transform& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Constraint> {
    void encode(const Constraint& var, IBlobWriter& writer);
    void decode(Constraint& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Texture> {
    void encode(const Texture& var, IBlobWriter& writer);
    void decode(Texture& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Material> {
    void encode(const Material& var, IBlobWriter& writer);
    void decode(Material& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Collider> {
    void encode(const Collider& var, IBlobWriter& writer);
    void decode(Collider& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Rigidbody> {
    void encode(const Rigidbody& var, IBlobWriter& writer);
    void decode(Rigidbody& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Sprite> {
    void encode(const Sprite& var, IBlobWriter& writer);
    void decode(Sprite& var, IBlobReader& reader);
};
template<>
struct SerialConvert<AnimatedSprite> {
    void encode(const AnimatedSprite& var, IBlobWriter& writer);
    void decode(AnimatedSprite& var, IBlobReader& reader);
};
template<>
struct SerialConvert<Model> {
    void encode(const Model& var, IBlobWriter& writer);
    void decode(Model& var, IBlobReader& reader);
};
// template<>
// struct SerialConvert<Model> {
//     void encode(const Model& var, IGlobWriter& writer);
//     void decode(Model& var, IGlobReader& reader);
// };

}
#endif
