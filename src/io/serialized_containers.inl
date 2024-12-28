#ifndef EMP_SERIALIZED_CONTAINERS
#define EMP_SERIALIZED_CONTAINERS
#include "serializer.hpp"
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
template<glm::length_t L, typename T>
struct SerialConvert<glm::vec<L, T>> {
    void encode(const glm::vec<L, T>& var, IGlobWriter& writer) {
        for(int i = 0; i < var.length(); i++) {
            writer.encode(var[i]);
        }
    }
    void decode(glm::vec<L, T>& var, IGlobReader& reader) {
        for(int i = 0; i < var.length(); i++) {
            reader.decode(var[i]);
        }
    }
};
template<glm::length_t C, glm::length_t R, typename T>
struct SerialConvert<glm::mat<C, R, T>> {
    void encode(const glm::mat<C, R, T>& var, IGlobWriter& writer) {
        for(int i = 0; i < C; i++) {
            for(int ii = 0; ii < R; ii++) {
                writer.encode(var[i][ii]);
            }
        }
    }
    void decode(glm::mat<C, R, T>& var, IGlobReader& reader) {
        for(int i = 0; i < C; i++) {
            for(int ii = 0; ii < R; ii++) {
                reader.decode(var[i][ii]);
            }
        }
    }
};

template<class T, size_t Size>
struct SerialConvert<std::array<T, Size>> {
    void encode(const std::array<T, Size>& var, IGlobWriter& writer) {
        for(int i = 0; i < Size; i++) {
            writer.encode(var[i]);
        }
    }
    void decode(std::array<T, Size>& var, IGlobReader& reader) {
        for(int i = 0; i < Size; i++) {
            reader.decode(var[i]);
        }
    }
};
template<class U>
struct SerialConvert<std::vector<U>> {
    void encode(const std::vector<U>& var, IGlobWriter& writer) {
        writer.encode(var.size());
        for(int i = 0; i < var.size(); i++) {
            writer.encode(var[i]);
        }
    }
    void decode(std::vector<U>& var, IGlobReader& reader) {
        size_t size;
        reader.decode(size);
        var.resize(size);
        for(int i = 0; i < var.size(); i++) {
            reader.decode(var[i]);
        }
    }
};
template<class T1, class T2>
struct SerialConvert<std::pair<T1, T2>> {
    void encode(const std::pair<T1, T2>& var, IGlobWriter& writer) {
        writer.encode(var.first);
        writer.encode(var.second);
    }
    void decode(std::pair<T1, T2>& var, IGlobReader& reader) {
        reader.decode(var.first);
        reader.decode(var.second);
    }
};
template<class Map>
void encodeMap(const Map& var, IGlobWriter& writer) {
    writer.encode(var.size());
    for(auto row : var) {
        writer.encode(row);
    }
}
template<class Map, class Key, class Value>
void decodeMap(Map& var, IGlobReader& reader) {
    size_t size;
    reader.decode(size);
    var.clear();
    for(int i = 0; i < size; i++) {
        std::pair<Key, Value> row;
        reader.decode(row);
        var.insert({row.first, row.second});
    }
}
template<class K, class V>
struct SerialConvert<std::map<K, V>> {
    void encode(const std::map<K, V>& var, IGlobWriter& writer) {
        encodeMap(var, writer);
    }
    void decode(std::map<K, V>& var, IGlobReader& reader) {
        decodeMap<std::map<K, V>, K, V>(var, reader);
    }
};
template<class K, class V>
struct SerialConvert<std::unordered_map<K, V>> {
    void encode(const std::unordered_map<K, V>& var, IGlobWriter& writer) {
        encodeMap(var, writer);
    }
    void decode(std::unordered_map<K, V>& var, IGlobReader& reader) {
        decodeMap<std::unordered_map<K, V>, K, V>(var, reader);
    }
};
template<class Set>
void encodeSet(const Set& var, IGlobWriter& writer) {
    writer.encode(var.size());
    for(auto key : var) {
        writer.encode(key);
    }
}
template<class Set, class Key>
void decodeSet(Set& var, IGlobReader& reader) {
    size_t size;
    reader.decode(size);
    var.clear();
    for(int i = 0; i < size; i++) {
        Key key;
        reader.decode(key);
        var.insert(key);
    }
}
template<class K>
struct SerialConvert<std::set<K>> {
    void encode(const std::set<K>& var, IGlobWriter& writer) {
        encodeSet(var, writer);
    }
    void decode(std::set<K>& var, IGlobReader& reader) {
        decodeSet<std::set<K>, K>(var, reader);
    }
};
template<class K>
struct SerialConvert<std::unordered_set<K>> {
    void encode(const std::unordered_set<K>& var, IGlobWriter& writer) {
        encodeSet(var, writer);
    }
    void decode(std::unordered_set<K>& var, IGlobReader& reader) {
        decodeSet<std::unordered_set<K>, K>(var, reader);
    }
};
}
#endif
