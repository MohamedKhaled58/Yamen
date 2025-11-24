#include "Core/Memory/PoolAllocator.h"
#include <cstdlib>
#include <algorithm>
#include <malloc.h> // For _aligned_malloc

namespace Yamen::Core {

// Helper for alignment
static constexpr size_t AlignUp(size_t value, size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

PoolAllocator::PoolAllocator(size_t objectSize, size_t objectAlignment, size_t initialCapacity)
    : m_ObjectSize(std::max(objectSize, sizeof(FreeNode)))
    , m_ObjectAlignment(objectAlignment)
    , m_Capacity(initialCapacity)
    , m_AllocatedCount(0) {
    
    // Align object size
    m_ObjectSize = AlignUp(m_ObjectSize, m_ObjectAlignment);
    
    // Allocate buffer
    const size_t bufferSize = m_ObjectSize * m_Capacity;
    
    // Use _aligned_malloc for Windows compatibility
    m_Buffer = static_cast<uint8_t*>(_aligned_malloc(bufferSize, m_ObjectAlignment));
    
    if (!m_Buffer) {
        throw std::bad_alloc();
    }
    
    // Initialize free list
    m_FreeList = nullptr;
    for (size_t i = 0; i < m_Capacity; ++i) {
        void* ptr = m_Buffer + (i * m_ObjectSize);
        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = m_FreeList;
        m_FreeList = node;
    }
}

PoolAllocator::~PoolAllocator() {
    if (m_Buffer) {
        _aligned_free(m_Buffer);
        m_Buffer = nullptr;
    }
}

PoolAllocator::PoolAllocator(PoolAllocator&& other) noexcept
    : m_Buffer(other.m_Buffer)
    , m_FreeList(other.m_FreeList)
    , m_ObjectSize(other.m_ObjectSize)
    , m_ObjectAlignment(other.m_ObjectAlignment)
    , m_Capacity(other.m_Capacity)
    , m_AllocatedCount(other.m_AllocatedCount) {
    
    other.m_Buffer = nullptr;
    other.m_FreeList = nullptr;
    other.m_Capacity = 0;
    other.m_AllocatedCount = 0;
}

PoolAllocator& PoolAllocator::operator=(PoolAllocator&& other) noexcept {
    if (this != &other) {
        if (m_Buffer) {
            _aligned_free(m_Buffer);
        }
        
        m_Buffer = other.m_Buffer;
        m_FreeList = other.m_FreeList;
        m_ObjectSize = other.m_ObjectSize;
        m_ObjectAlignment = other.m_ObjectAlignment;
        m_Capacity = other.m_Capacity;
        m_AllocatedCount = other.m_AllocatedCount;
        
        other.m_Buffer = nullptr;
        other.m_FreeList = nullptr;
        other.m_Capacity = 0;
        other.m_AllocatedCount = 0;
    }
    return *this;
}

void* PoolAllocator::Allocate() {
    if (!m_FreeList) {
        Grow();
    }
    
    if (!m_FreeList) {
        return nullptr;
    }
    
    FreeNode* node = m_FreeList;
    m_FreeList = node->next;
    m_AllocatedCount++;
    
    return node;
}

void PoolAllocator::Free(void* ptr) noexcept {
    if (!ptr) {
        return;
    }
    
    FreeNode* node = static_cast<FreeNode*>(ptr);
    node->next = m_FreeList;
    m_FreeList = node;
    m_AllocatedCount--;
}

void PoolAllocator::Grow() {
    const size_t newCapacity = m_Capacity * 2;
    const size_t newBufferSize = m_ObjectSize * newCapacity;
    
    uint8_t* newBuffer = static_cast<uint8_t*>(_aligned_malloc(newBufferSize, m_ObjectAlignment));
    if (!newBuffer) {
        return;
    }
    
    // Copy old buffer
    if (m_Buffer) {
        std::memcpy(newBuffer, m_Buffer, m_ObjectSize * m_Capacity);
        _aligned_free(m_Buffer);
    }
    
    m_Buffer = newBuffer;
    
    // Add new objects to free list
    for (size_t i = m_Capacity; i < newCapacity; ++i) {
        void* ptr = m_Buffer + (i * m_ObjectSize);
        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = m_FreeList;
        m_FreeList = node;
    }
    
    m_Capacity = newCapacity;
}

} // namespace Yamen::Core
