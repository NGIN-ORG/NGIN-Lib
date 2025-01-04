#pragma once
#include <cstddef>
#include <cstdint>

namespace NGIN
{
    /// @brief Represents a 64-bit unsigned integer.
    using UInt64 = std::uint64_t;
    /// @brief Represents a 32-bit unsigned integer.
    using UInt32 = std::uint32_t;
    /// @brief Represents a 16-bit unsigned integer.
    using UInt16 = std::uint16_t;
    /// @brief Represents an 8-bit unsigned integer.
    using UInt8 = std::uint8_t;

    /// @brief Represents a 64-bit signed integer.
    using Int64 = std::int64_t;
    /// @brief Represents a 32-bit signed integer.
    using Int32 = std::int32_t;
    /// @brief Represents a 16-bit signed integer.
    using Int16 = std::int16_t;
    /// @brief Represents an 8-bit signed integer.
    using Int8 = std::int8_t;

    /// @brief Represents a byte.
    using Byte = std::byte;

    /// @brief Represents a 32-bit floating point number.
    using F32 = float;
    /// @brief Represents a 64-bit floating point number.
    using F64 = double;

    /// @brief Represents a signed integer type that is large enought to hold a pointer.
    using IntPtr = std::intptr_t;
    /// @brief Represents an unsigned integer type that is large enought to hold a pointer.
    using UIntPtr = std::uintptr_t;

    /// @brief Represents a character type, usually 8-bit.
    using Char = char;
    /// @brief Represents a wide character type, depends on the platform. Most probably it will either be 16-bit or 32-bit.
    using WChar = wchar_t;
    /// @brief Represents a 8-bit character type.
    using Char8 = char8_t;
    /// @brief Represents a 16-bit character type.
    using Char16 = char16_t;
    /// @brief Represents a 32-bit character type.
    using Char32 = char32_t;

    /// @brief Represents a signed integer.
    using Int = int;
    /// @brief Represents an unsigned integer.
    using UInt = unsigned int;
    /// @brief Represents a long signed integer.
    using Long = long;
    /// @brief Represents a long unsigned integer.
    using ULong = unsigned long;
}// namespace NGIN
