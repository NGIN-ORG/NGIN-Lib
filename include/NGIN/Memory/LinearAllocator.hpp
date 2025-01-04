/// @file LinearAllocator.hpp
/// @brief Declaration of the LinearAllocator class, an arena-style allocator.
///
/// @details
/// A LinearAllocator manages a single contiguous block of memory in a linear fashion.
/// Allocation is fast and sequential, but no per-allocation deallocation is supported.
/// You can only free all allocations at once by calling `Reset()`.

#pragma once

#include <NGIN/Memory/Mallocator.hpp>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace NGIN::Memory
{

    class LinearAllocator : private Mallocator
    {
    public:
        LinearAllocator() = delete;
        LinearAllocator(MemoryBlock block);
        LinearAllocator(std::size_t capacity);
        LinearAllocator(std::size_t capacity, IAllocator& allocator);

        LinearAllocator(const LinearAllocator&) = delete;
        LinearAllocator& operator=(const LinearAllocator&) = delete;

        LinearAllocator(LinearAllocator&&);
        LinearAllocator& operator=(LinearAllocator&&);

        virtual ~LinearAllocator() = default;

        /// @brief Allocates a block of memory of the given size and alignment.
        /// @param size The size of the block in bytes.
        /// @param alignment The required alignment (must be a power of two).
        /// @return Pointer to the allocated memory.
        MemoryBlock Allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) override;



    private:
        void* m_memory = nullptr; /// Pointer to the memory block.
        std::size_t m_remainingCapacity = 0; /// Remaining capacity in the block.
    };
} // namespace NGIN::Memory
