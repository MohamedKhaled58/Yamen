#include "Core/Memory/LinearAllocator.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace Yamen::Core {

LinearAllocator::LinearAllocator(size_t capacity)
    : m_Capacity(capacity)
    , m_Offset(0) {
    m_Buffer = static_cast<uint8_t*>(std::malloc(capacity));
    if (!m_Buffer) {
        throw std::bad_alloc();
    }
}

LinearAllocator::~LinearAllocator() {
    if (m_Buffer) {
        std::free(m_Buffer);
        m_Buffer = nullptr;
    }
}

LinearAllocator::LinearAllocator(LinearAllocator&& other) noexcept
    : m_Buffer(other.m_Buffer)
    , m_Capacity(other.m_Capacity)
    , m_Offset(other.m_Offset) {
    other.m_Buffer = nullptr;
    other.m_Capacity = 0;
    other.m_Offset = 0;
}

LinearAllocator& LinearAllocator::operator=(LinearAllocator&& other) noexcept {
    if (this != &other) {
        if (m_Buffer) {
            std::free(m_Buffer);
        }
        
        m_Buffer = other.m_Buffer;
        m_Capacity = other.m_Capacity;
        m_Offset = other.m_Offset;
        
        other.m_Buffer = nullptr;
        other.m_Capacity = 0;
        other.m_Offset = 0;
    }
    return *this;
}

void* LinearAllocator::Allocate(size_t size, size_t alignment) {
    if (size == 0) {
        return nullptr;
    }
    
    // Align current offset
    const size_t alignedOffset = AlignUp(m_Offset, alignment);
    
    // Check if we have enough space
    if (alignedOffset + size > m_Capacity) {
        return nullptr;
    }
    
    // Allocate
    void* ptr = m_Buffer + alignedOffset;
    m_Offset = alignedOffset + size;
    
    return ptr;
}

void LinearAllocator::Reset() noexcept {
    m_Offset = 0;
}

bool LinearAllocator::CanAllocate(size_t size, size_t alignment) const noexcept {
    const size_t alignedOffset = AlignUp(m_Offset, alignment);
    return alignedOffset + size <= m_Capacity;
}

} // namespace Yamen::Core
