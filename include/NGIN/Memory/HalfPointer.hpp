// HalfPointer.hpp
#pragma once

#include <cstdint>
#include <cassert>

namespace NGIN::Memory
{
    class HalfPointer
    {
    public:
        static constexpr std::uint32_t INVALID_OFFSET = 0xFFFFFFFF;
    public:
        HalfPointer() : offset(INVALID_OFFSET) {} // Invalid pointer
        HalfPointer(void* base, void* ptr)
        {
            assert(ptr >= base && "Pointer must be within the heap");
            offset = static_cast<std::uint32_t>(reinterpret_cast<char*>(ptr) - reinterpret_cast<char*>(base));
        }

        void* ToAbsolute(void* base) const
        {
            if (offset == INVALID_OFFSET) return nullptr;
            return reinterpret_cast<char*>(base) + offset;
        }

        std::uint32_t GetOffset() const { return offset; }

    private:
        std::uint32_t offset;
    };
}
