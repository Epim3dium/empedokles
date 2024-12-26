#ifndef EMP_SERIALIZER_HPP
#define EMP_SERIALIZER_HPP
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "math/shapes/AABB.hpp"
namespace emp {
typedef uint8_t byte;
class IGlobWriter {
    uint32_t m_version = 0;
public:
    uint32_t version() const { return m_version; }

    virtual void copy(const void* source, size_t size) = 0;

    template<class T>
    void encode(const T& var);
    virtual ~IGlobWriter() {}
    IGlobWriter(uint32_t version) : m_version(version) {}
};
class IGlobReader {
    uint32_t m_version = 0;
public:
    uint32_t version() const { return m_version; }

    virtual void* get(size_t size) = 0;

    template<class T>
    void decode(T& var);
    virtual ~IGlobReader() {}
    IGlobReader(uint32_t version) : m_version(version) {}
};
class Glob : public IGlobWriter, public IGlobReader {
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
    Glob() : IGlobWriter(current_version), IGlobReader(current_version) {
        //load to data current version
        Header header {current_version};
        encode(header);
        decode(header);
    }
};
template<class T>
struct SerialConvert {
    static_assert(std::is_trivially_copyable_v<T> == true && std::is_fundamental<T>::value == true);
    void encode(const T& var, IGlobWriter& writer) {
        writer.copy(&var, sizeof(T));
    }
    void decode(T& var, IGlobReader& reader) {
        var = *(T*)reader.get(sizeof(T));
    }
};
template<class T>
void IGlobWriter::encode(const T& var) {
    SerialConvert<T>().encode(var, *this);
}
template<class T>
void IGlobReader::decode(T& var) {
    SerialConvert<T>().decode(var, *this);
}

template<>
struct SerialConvert<Glob::Header> {
    void encode(const Glob::Header& var, IGlobWriter& writer);
    void decode(Glob::Header& var, IGlobReader& reader);
};

template<>
struct SerialConvert<AABB> {
    void encode(const AABB& var, IGlobWriter& writer);
    void decode(AABB& var, IGlobReader& reader);
};
template<>
struct SerialConvert<std::string> {
    void encode(const std::string& var, IGlobWriter& writer);
    void decode(std::string& var, IGlobReader& reader);
};

}
#include "serialized_containers.inl"
#endif
