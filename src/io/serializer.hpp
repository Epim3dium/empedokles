#ifndef EMP_SERIALIZER_HPP
#define EMP_SERIALIZER_HPP
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <set>
#include "debug/log.hpp"
#include "serialized_containers.hpp"
#include "serialized_components.hpp"

#include <type_traits>
#include <vector>
#include "serial_convert.hpp"
#include "core/coordinator.hpp"
#include "core/entity.hpp"
#include "math/shapes/AABB.hpp"
#include "scene/register_scene_types.hpp"
namespace emp {
class Blob : public IBlobWriter, public IBlobReader {
    size_t read_next = 0;
    std::vector<byte> data;
    static constexpr uint32_t current_version =  1;
public:
    struct Header {
        uint32_t version;
        char reserved[12];
    };
    void copy(const void* source, size_t size) override;
    void* get(size_t size) override;

    //initialize to current version if initializing from memory
    Blob() : IBlobWriter(current_version), IBlobReader(current_version) {
        //load to data current version
        Header header {current_version};
        encode(header);
        decode(header);
    }
};


template<>
struct SerialConvert<Blob::Header> {
    void encode(const Blob::Header& var, IBlobWriter& writer);
    void decode(Blob::Header& var, IBlobReader& reader);
};

template<>
struct SerialConvert<AABB> {
    void encode(const AABB& var, IBlobWriter& writer);
    void decode(AABB& var, IBlobReader& reader);
};
template<>
struct SerialConvert<std::string> {
    void encode(const std::string& var, IBlobWriter& writer);
    void decode(std::string& var, IBlobReader& reader);
};
template<typename T>
struct SerialConvert<std::optional<T>> {
    void encode(const std::optional<T>& var, IBlobWriter& writer) {
        bool has_value = var.has_value();
        writer.encode(has_value);
        if (!has_value) {
            return;
        }
        writer.encode(var.value());
    }
    void decode(std::optional<T>& var, IBlobReader& reader) {
        bool has_value;
        reader.decode(has_value);
        if(!has_value) {
            return;
        }
        T temp;
        reader.decode(temp);
        var = std::move(temp);
    }
};
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
        std::cerr << var.entities.size() << "\n";
        for(auto e_prev : var.entities) {
            auto e = var.ECS.createEntity();
            decodeAll(var.ECS, e, reader, AllComponentTypes());
        }
    }
};

}
#endif
