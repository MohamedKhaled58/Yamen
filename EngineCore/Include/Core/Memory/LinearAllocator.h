#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>

namespace Yamen::Core {

/**
 * @brief Linear allocator for fast sequential allocations
 * 
 * Allocates memory in a linear fashion from a pre-allocated buffer.
 * Very fast allocation, but can only free all at once.
 * Perfect for per-frame allocations or temporary data.
 */
class LinearAllocator {
public:
    explicit LinearAllocator(size_t capacity);
    ~LinearAllocator();

    // Non-copyable
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;

    // Movable
    LinearAllocator(LinearAllocator&& other) noexcept;
    LinearAllocator& operator=(LinearAllocator&& other) noexcept;

    /**
     * @brief Allocate memory with alignment
     * @param size Size in bytes
     * @param alignment Alignment requirement (must be power of 2)
     * @return Pointer to allocated memory, or nullptr if out of space
     */
    [[nodiscard]] void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t));

    /**
     * @brief Reset allocator to beginning (frees all allocations)
     */
    void Reset() noexcept;

    /**
     * @brief Get current usage
     */
    [[nodiscard]] size_t GetUsedSize() const noexcept { return m_Offset; }

    /**
     * @brief Get total capacity
     */
    [[nodiscard]] size_t GetCapacity() const noexcept { return m_Capacity; }

    /**
     * @brief Check if allocator has space for allocation
     */
    [[nodiscard]] bool CanAllocate(size_t size, size_t alignment = alignof(std::max_align_t)) const noexcept;

private:
    uint8_t* m_Buffer = nullptr;
    size_t m_Capacity = 0;
    size_t m_Offset = 0;
};

/**
 * @brief Align a value up to the nearest multiple of alignment
 */
constexpr size_t AlignUp(size_t value, size_t alignment) noexcept {
    return (value + alignment - 1) & ~(alignment - 1);
}

/**
 * @brief Check if a value is aligned
 */
constexpr bool IsAligned(size_t value, size_t alignment) noexcept {
    return (value & (alignment - 1)) == 0;
}

/**
 * @brief Check if a pointer is aligned
 */
inline bool IsAligned(const void* ptr, size_t alignment) noexcept {
    return IsAligned(reinterpret_cast<uintptr_t>(ptr), alignment);
}

} // namespace Yamen::Core
