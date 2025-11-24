#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Yamen::Core {

/**
 * @brief Pool allocator for fixed-size object allocations
 * 
 * Allocates objects of a fixed size from a pool.
 * Very fast allocation and deallocation.
 * Perfect for frequently allocated/deallocated objects of the same type.
 */
class PoolAllocator {
public:
    /**
     * @brief Construct pool allocator
     * @param objectSize Size of each object in bytes
     * @param objectAlignment Alignment requirement
     * @param initialCapacity Initial number of objects
     */
    PoolAllocator(size_t objectSize, size_t objectAlignment, size_t initialCapacity = 64);
    ~PoolAllocator();

    // Non-copyable
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    // Movable
    PoolAllocator(PoolAllocator&& other) noexcept;
    PoolAllocator& operator=(PoolAllocator&& other) noexcept;

    /**
     * @brief Allocate one object from the pool
     * @return Pointer to allocated memory
     */
    [[nodiscard]] void* Allocate();

    /**
     * @brief Free an object back to the pool
     * @param ptr Pointer to free (must have been allocated from this pool)
     */
    void Free(void* ptr) noexcept;

    /**
     * @brief Get object size
     */
    [[nodiscard]] size_t GetObjectSize() const noexcept { return m_ObjectSize; }

    /**
     * @brief Get number of allocated objects
     */
    [[nodiscard]] size_t GetAllocatedCount() const noexcept { return m_AllocatedCount; }

    /**
     * @brief Get total capacity
     */
    [[nodiscard]] size_t GetCapacity() const noexcept { return m_Capacity; }

private:
    void Grow();

    struct FreeNode {
        FreeNode* next;
    };

    uint8_t* m_Buffer = nullptr;
    FreeNode* m_FreeList = nullptr;
    size_t m_ObjectSize = 0;
    size_t m_ObjectAlignment = 0;
    size_t m_Capacity = 0;
    size_t m_AllocatedCount = 0;
};

} // namespace Yamen::Core
