#include "serializer.hpp"
#include <unistd.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include "math/shapes/AABB.hpp"
namespace emp {
void Blob::copy(const void* source, size_t size) {
    auto start = data.size();
    data.resize(data.size() + size);
    memcpy(&data[start], source, size);
}
void* Blob::get(size_t size) {
    assert(read_next < data.size() && "trying to read out of serializer data range");
    void* result = &data[read_next];
    read_next += size;
    return result;
}
// #define SIMPLE_ENCODE_DECODE(type)\
// void SerialConvert<type>::encode(const type& var, IGlobWriter& writer) {\
//     writer.copy(&var, sizeof(type));\
// }\
// void SerialConvert<type>::decode(type& var, IGlobReader& reader) {\
//     var = *(type*)reader.get(sizeof(type));\
// }

void SerialConvert<Blob::Header>::encode(const Blob::Header& var, IBlobWriter& writer) {
    writer.copy(&var, sizeof(Blob::Header));
}
void SerialConvert<Blob::Header>::decode(Blob::Header& var, IBlobReader& reader) {
    typedef Blob::Header Hdr;
    var = *(Hdr*)reader.get(sizeof(Hdr));
}
void SerialConvert<AABB>::encode(const AABB& var, IBlobWriter& writer) {
    writer.encode(var.min);
    writer.encode(var.max);
}
void SerialConvert<AABB>::decode(AABB& var, IBlobReader& reader) {
    reader.decode(var.min);
    reader.decode(var.max);
}
void SerialConvert<std::string>::encode(const std::string& var, IBlobWriter& writer) {
    writer.encode(var.size());
    for(int i = 0; i < var.size(); i++) {
        writer.encode(var[i]);
    }
}
void SerialConvert<std::string>::decode(std::string& var, IBlobReader& reader) {
    size_t size;
    reader.decode(size); 
    var.resize(size);
    for(int i = 0; i < var.size(); i++) {
        reader.decode(var[i]);
    }
}
}
