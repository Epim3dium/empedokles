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

}
#endif
