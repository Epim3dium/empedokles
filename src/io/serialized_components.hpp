#ifndef EMP_SERIALIZED_COMPONENTS_HPP
#define EMP_SERIALIZED_COMPONENTS_HPP
#include "graphics/animated_sprite.hpp"
#include "graphics/model.hpp"
#include "graphics/sprite.hpp"
#include "physics/collider.hpp"
#include "physics/constraint.hpp"
#include "physics/material.hpp"
#include "scene/register_scene_types.hpp"
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
struct SerialConvert<MovingSprite> {
    void encode(const MovingSprite& var, IBlobWriter& writer);
    void decode(MovingSprite& var, IBlobReader& reader);
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
template<typename T, typename = void>
struct is_iterable : std::false_type {};
template<typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>())),
                                  decltype(std::end(std::declval<T>()))>> 
    : std::true_type {};
template<typename T>
struct stores_entity : std::is_same<typename T::value_type, Entity> {};
template<typename T>
constexpr bool is_iterable_and_stores_entity = 
    is_iterable<T>::value && stores_entity<T>::value;

template<typename Container_t>
struct EntityRange {
    Coordinator& ECS;
    Container_t& entities;
    static_assert(is_iterable_and_stores_entity<Container_t>, "Type Container_t must store entities");
};
template<typename C>
struct SerialConvert<EntityRange<C>> {
    template<class T>
    void encodeComponent(Coordinator& ECS, Entity owner, IBlobWriter& writer) {
        bool ownership = false;
        if(ECS.hasComponent<T>(owner)) {
            ownership = true;
        }
        writer.encode(ownership);
        if(!ownership) {
            return;
        }

        T& component = *ECS.getComponent<T>(owner);
        writer.encode(component);
    }
    template<class T>
    void decodeComponent(Coordinator& ECS, Entity owner, IBlobReader& reader) {
        bool ownership = false;
        reader.decode(ownership);
        if(!ownership) {
            return;
        }

        T component;
        reader.decode(component);
        ECS.addComponent(owner, component);
    }
    template <class... CompTs>
    void encodeAll(Coordinator& ECS,
        Entity owner,
        IBlobWriter& writer,
        TypePack<CompTs...>) 
    {
        (encodeComponent<CompTs>(ECS, owner, writer), ...);
    }
    template <class... CompTs>
    void decodeAll(Coordinator& ECS,
        Entity owner,
        IBlobReader& reader,
        TypePack<CompTs...>) 
    {
        (decodeComponent<CompTs>(ECS, owner, reader), ...);
    }

    void encode(const EntityRange<C>& var, IBlobWriter& writer) {
        writer.encode(var.entities);
        for(auto e : var.entities) {
            encodeAll(var.ECS, e, writer, AllComponentTypes());
        }
    }
    void decode(EntityRange<C>& var, IBlobReader& reader) {
        var.entities.clear();
        reader.decode(var.entities);
        for(auto& e_ref : var.entities) {
            auto previous_entity = e_ref;
            auto entity = var.ECS.createEntity();
            decodeAll(var.ECS, entity, reader, AllComponentTypes());
            e_ref = entity;
        }
    }
};

}
#endif
