// Mallocator.hpp
#pragma once

#include <cstddef>
#include <stdlib.h>

#include <NGIN/Memory/IAllocator.hpp>

namespace NGIN::Memory
{
    /// @class Mallocator
    /// @brief An allocator that uses platform-specific aligned memory allocation functions.
    ///
    /// @details
    /// `Mallocator` is designed as a general-purpose allocator that provides aligned memory
    /// allocations and deallocations. It conforms to the `IAllocator` interface, making it
    /// compatible with composable allocator decorators.
    ///
    /// @note
    /// - The `Reset` method is a no-op since `Mallocator` does not track allocations.
    /// - The `Owns` method always returns `true` as `Mallocator` cannot track ownership and assumes it owns it.
    class Mallocator : public IAllocator
    {
    public:
        /// @brief Allocates a block of memory with the specified size and alignment.
        ///
        /// @param size The size of the memory block to allocate, in bytes.
        /// @param alignment The alignment requirement of the memory block, in bytes.
        ///                  Must be a non-zero power of two.
        ///
        /// @return A pointer to the beginning of the allocated memory block.
        ///
        /// @throws std::invalid_argument If the alignment is not a non-zero power of two.
        /// @throws std::bad_alloc If the memory allocation fails.
        inline virtual MemoryBlock Allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t))
        {
// Platform-specific aligned allocation
#if defined(_WIN32) || defined(_WIN64)
            return {_aligned_malloc(size, alignment), size};
#elif defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
            // posix_memalign requires alignment to be a multiple of sizeof(void*) and a power of two
            void* ptr;
            posix_memalign(&ptr, alignment, size);
            return {ptr, size};
#elif __cplusplus >= 201703L
            // Fallback to std::aligned_alloc if available (C++17 and later)
            // Note: std::aligned_alloc requires size to be a multiple of alignment
            if (size % alignment != 0)
                size += alignment - (size % alignment);
            return {std::aligned_alloc(alignment, size), size};
#else
            static_assert(false, "Aligned memory allocation not supported on this platform");
#endif
        }

        /// @brief Deallocates a previously allocated memory block.
        ///
        /// @param ptr A pointer to the memory block to deallocate.
        ///
        /// @note
        /// - The pointer must have been returned by a prior call to `Allocate`.
        /// - After deallocation, the pointer is invalid and must not be used.
        inline virtual void Deallocate(void* ptr) noexcept override
        {
            if (!ptr)
                return;

// Platform-specific deallocation
#if defined(_WIN32) || defined(_WIN64)
            _aligned_free(ptr);
#elif defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
            free(ptr);
#elif __cplusplus >= 201703L
            // std::aligned_alloc is deallocated using free
            std::free(ptr);
#else
            static_assert(false, "Aligned memory deallocation not supported on this platform");
#endif
        }

        /// @brief Resets the allocator to its initial state.
        ///
        /// @details
        /// This function is a no-op for `Mallocator` as it does not track allocations.
        inline virtual void Reset() noexcept override
        {
            // No operation needed for Mallocator
            return;
        }

        /// @brief Checks if the allocator owns the given memory block.
        ///
        /// @param ptr A pointer to the memory block to check.
        /// @return Always returns `true` as `Mallocator` cannot track ownership.
        inline virtual bool Owns(const void* ptr) const noexcept override
        {
            // Mallocator does not track ownership; cannot determine if it owns the pointer
            (void) ptr; // Suppress unused parameter warning
            return true;
        }

        inline virtual std::size_t GetCapacity() const noexcept override
        {
            return 0;
        }

        inline virtual std::size_t GetUsedSize() const noexcept override
        {
            return 0;
        }

    };
}// namespace NGIN::Memory
