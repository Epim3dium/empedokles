#include "linear_allocator.hpp"
/* Constructors */

namespace emp {
LinearAllocator::LinearAllocator(size_t total_size) : Allocator(total_size), m_size(total_size), m_free(total_size), m_preAlloc(false), m_initialized(false) {
    m_start = (uint8_t*)malloc(m_size * sizeof(uint8_t));
    m_end = (void*)((uintptr_t)m_start + m_size * sizeof(uint8_t));
    m_cur = m_start;
    m_initialized = true;
}

LinearAllocator::LinearAllocator(void* buffer, size_t total_size) : Allocator(total_size), m_size(total_size), m_free(total_size), m_preAlloc(true), m_initialized(true) {
    m_start = buffer;
    m_end = (void*)((uintptr_t)m_start + m_size * sizeof(uint8_t));
}

/* Destructor */

LinearAllocator::~LinearAllocator() {
    if (m_preAlloc || !m_initialized)
        return;
    free(m_start);
}

void* LinearAllocator::Allocate(size_t sz, size_t alignment) {
    if (!m_initialized)
        return nullptr;

    assert((alignment & (alignment - 1)) == 0);

    void* ptr = std::align(alignment, sz, m_cur, m_free);

    if (!ptr) {
        assert(false && "Linear allocator full!");
        return nullptr;
    }

    m_free -= sz;
    m_cur = (void*)((uintptr_t)ptr+sz);


    return ptr;
}

void LinearAllocator::Free(void*) {
    // not supported for linear allocator
}

void LinearAllocator::Release() {
    if (m_preAlloc)
        return;
    free(m_start);
    m_initialized = false;
}

void LinearAllocator::Reset() {
    m_cur = m_start;
}

void LinearAllocator::ZeroMem() {
    memset(m_start, 0, m_size);
}

void LinearAllocator::Layout() {
}

}
