#ifndef EMP_SERIALIZER_HPP
#define EMP_SERIALIZER_HPP
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include "math/math_defs.hpp"
#include "math/shapes/AABB.hpp"
namespace emp {
typedef uint8_t byte;
class IGlobWriter {
    bool m_isMappable = true;
    uint32_t m_version = 0;
public:
    uint32_t version() const { return m_version; }
    bool isMappable() const { return m_isMappable; }
    void markNotMappable() { m_isMappable = false; }

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
    void copy(const void* source, size_t size) override;
    void* get(size_t size) override;

    //initialize to current version if initializing from memory
    Glob() : IGlobWriter(current_version), IGlobReader(current_version) {
        //load to data current version
        uint32_t dummy;
        encode(current_version);
        decode(dummy);
    }
};
template<class T>
struct SerialConvert {
    static_assert(false);
    void encode(const T& var, IGlobWriter& writer) {}
    void decode(T& var, IGlobReader& reader) {}
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
struct SerialConvert<float> {
    void encode(const float& var, IGlobWriter& writer);
    void decode(float& var, IGlobReader& reader);
};
template<>
struct SerialConvert<int32_t> {
    void encode(const int32_t& var, IGlobWriter& writer);
    void decode(int32_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<uint32_t> {
    void encode(const uint32_t& var, IGlobWriter& writer);
    void decode(uint32_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<int64_t> {
    void encode(const int64_t& var, IGlobWriter& writer);
    void decode(int64_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<uint64_t> {
    void encode(const uint64_t& var, IGlobWriter& writer);
    void decode(uint64_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<int8_t> {
    void encode(const int8_t& var, IGlobWriter& writer);
    void decode(int8_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<uint8_t> {
    void encode(const uint8_t& var, IGlobWriter& writer);
    void decode(uint8_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<char> {
    void encode(const char& var, IGlobWriter& writer);
    void decode(char& var, IGlobReader& reader);
};
template<>
struct SerialConvert<size_t> {
    void encode(const size_t& var, IGlobWriter& writer);
    void decode(size_t& var, IGlobReader& reader);
};
template<>
struct SerialConvert<bool> {
    void encode(const bool& var, IGlobWriter& writer);
    void decode(bool& var, IGlobReader& reader);
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
