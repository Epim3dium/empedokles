#ifndef EMP_SET_H
#define EMP_SET_H
#include <_types/_uint32_t.h>
#include <set>
#include <string>

namespace emp {

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T>>
struct Set : public std::set<T, CMP, ALLOC> {
    operator bool() const {
        return this->size() != 0;
    }
    Set operator^(const std::set<T, CMP, ALLOC>& s2) const;
    Set operator+(const std::set<T, CMP, ALLOC>& s2) const;

    // Set(const std::set<T>& other) = default;
    // Set(std::set<T>&& other) = default;
    // Set& operator=(const std::set<T>& other) = default;
    // Set& operator=(std::set<T>&& other) = default;
};
// implementation of templates
template <class T, class CMP, class ALLOC>
Set<T, CMP, ALLOC> Set<T, CMP, ALLOC>::operator^(
        const std::set<T, CMP, ALLOC>& s2
) const {
    Set<T, CMP, ALLOC> s;
    std::set_intersection(
            this->begin(),
            this->end(),
            s2.begin(),
            s2.end(),
            std::inserter(s, s.begin())
    );
    return s;
}
template <class T, class CMP, class ALLOC>
Set<T, CMP, ALLOC> Set<T, CMP, ALLOC>::operator+(
        const std::set<T, CMP, ALLOC>& s2
) const {
    Set<T, CMP, ALLOC> s;
    std::set_union(
            this->begin(),
            this->end(),
            s2.begin(),
            s2.end(),
            std::inserter(s, s.begin())
    );
    return s;
}

} // namespace emp
#endif // EMP_SET_H
