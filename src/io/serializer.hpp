#ifndef EMP_SERIALIZER_HPP
#define EMP_SERIALIZER_HPP
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "math/shapes/AABB.hpp"
namespace emp {
typedef uint8_t byte;
class IBlobWriter {
    uint32_t m_version = 0;
public:
    uint32_t version() const { return m_version; }

    virtual void copy(const void* source, size_t size) = 0;

    template<class T>
    void encode(const T& var);
    virtual ~IBlobWriter() {}
    IBlobWriter(uint32_t version) : m_version(version) {}
};
class IBlobReader {
    uint32_t m_version = 0;
public:
    uint32_t version() const { return m_version; }

    virtual void* get(size_t size) = 0;

    template<class T>
    void decode(T& var);
    virtual ~IBlobReader() {}
    IBlobReader(uint32_t version) : m_version(version) {}
};
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
template<class T>
struct SerialConvert {
    static_assert(std::is_trivially_copyable_v<T> == true && std::is_fundamental<T>::value == true);
    void encode(const T& var, IBlobWriter& writer) {
        writer.copy(&var, sizeof(T));
    }
    void decode(T& var, IBlobReader& reader) {
        var = *(T*)reader.get(sizeof(T));
    }
};
template<class T>
void IBlobWriter::encode(const T& var) {
    SerialConvert<T>().encode(var, *this);
}
template<class T>
void IBlobReader::decode(T& var) {
    SerialConvert<T>().decode(var, *this);
}

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
        var = temp;
    }
};

}
#include "serialized_containers.inl"
#endif
