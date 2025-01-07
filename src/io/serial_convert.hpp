#ifndef EMP_SERIAL_CONVERT_HPP
#define EMP_SERIAL_CONVERT_HPP
#include <type_traits>
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

template<class T>
struct SerialConvert {
    static_assert(std::is_trivially_copyable_v<T> == true && std::is_fundamental<T>::value == true);
    static_assert(std::is_pointer<T>::value == false, "serializing raw pointers is not safe");
    void encode(const T& var, IBlobWriter& writer) {
        writer.copy(&var, sizeof(T));
    }
    void decode(T& var, IBlobReader& reader) {
        var = *(T*)reader.get(sizeof(T));
    }
};

}

#endif // !DEBUG
