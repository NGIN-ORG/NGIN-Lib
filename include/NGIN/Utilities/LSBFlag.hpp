#pragma once

#include <type_traits>
#include <cstdint>
#include <limits>

namespace NGIN::Utilities
{
    /// @class LSBFlag
    /// @brief A utility class that stores a flag in the least significant bit of an unsigned integral type.
    ///
    /// This class encapsulates the bit manipulation required to store and retrieve a flag from the LSB
    /// of an integral value. It ensures type safety and provides a clean interface for working with
    /// the value and the flag separately.
    template<typename T>
    class LSBFlag
    {
    public:
        static_assert(std::is_integral<T>::value, "LSBFlag can only be used with integral types.");
        static_assert(std::is_unsigned<T>::value, "LSBFlag requires an unsigned integral type.");

        /// Default constructor initializes value and flag to zero.
        constexpr LSBFlag() noexcept;

        /// Constructor that initializes with a value and a flag.
        /// @param value The value to store.
        /// @param flag The flag to store (true or false).
        constexpr LSBFlag(T value, bool flag) noexcept;

        /// Sets the value.
        /// @param value The new value to store.
        void SetValue(T value) noexcept;

        /// Retrieves the value.
        /// @return The stored value.
        constexpr T GetValue() const noexcept;

        /// Sets the flag.
        /// @param flag The new flag value.
        void SetFlag(bool flag) noexcept;

        /// Retrieves the flag.
        /// @return The stored flag.
        constexpr bool GetFlag() const noexcept;

        /// Sets both the value and the flag.
        /// @param value The new value.
        /// @param flag The new flag.
        void Set(T value, bool flag) noexcept;

        /// Retrieves the raw data (value and flag combined).
        /// @return The combined data.
        constexpr T GetRaw() const noexcept;

        /// Sets the raw data directly.
        /// @param data The raw data to set.
        void SetRaw(T data) noexcept;

        /// Equality operator.
        constexpr bool operator==(const LSBFlag& other) const noexcept;

        /// Inequality operator.
        constexpr bool operator!=(const LSBFlag& other) const noexcept;

    private:
        T data;
    };

    // Implementation of LSBFlag

    template<typename T>
    inline constexpr LSBFlag<T>::LSBFlag() noexcept
        : data(0)
    {
    }

    template<typename T>
    inline constexpr LSBFlag<T>::LSBFlag(T value, bool flag) noexcept
        : data((value << 1) | static_cast<T>(flag))
    {
    }

    template<typename T>
    inline void LSBFlag<T>::SetValue(T value) noexcept
    {
        data = (value << 1) | (data & static_cast<T>(1));
    }

    template<typename T>
    inline constexpr T LSBFlag<T>::GetValue() const noexcept
    {
        return data >> 1;
    }

    template<typename T>
    inline void LSBFlag<T>::SetFlag(bool flag) noexcept
    {
        data = (data & ~static_cast<T>(1)) | static_cast<T>(flag);
    }

    template<typename T>
    inline constexpr bool LSBFlag<T>::GetFlag() const noexcept
    {
        return static_cast<bool>(data & static_cast<T>(1));
    }

    template<typename T>
    inline void LSBFlag<T>::Set(T value, bool flag) noexcept
    {
        data = (value << 1) | static_cast<T>(flag);
    }

    template<typename T>
    inline constexpr T LSBFlag<T>::GetRaw() const noexcept
    {
        return data;
    }

    template<typename T>
    inline void LSBFlag<T>::SetRaw(T data) noexcept
    {
        this->data = data;
    }

    template<typename T>
    inline constexpr bool LSBFlag<T>::operator==(const LSBFlag& other) const noexcept
    {
        return data == other.data;
    }

    template<typename T>
    inline constexpr bool LSBFlag<T>::operator!=(const LSBFlag& other) const noexcept
    {
        return data != other.data;
    }

}// namespace NGIN::Utilities
