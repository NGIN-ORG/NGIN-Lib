#pragma once

#include <type_traits>
#include <cstdint>
#include <limits>

namespace NGIN::Utilities
{
    /// @class MSBFlag
    /// @brief A utility class that stores a flag in the most significant bit of an unsigned integral type.
    ///
    /// This class encapsulates the bit manipulation required to store and retrieve a flag from the MSB
    /// of an unsigned integral value. It ensures type safety and provides a clean interface for working with
    /// the value and the flag separately.
    template<typename T>
    class MSBFlag
    {
    public:
        // Ensure that T is an unsigned integral type
        static_assert(std::is_integral<T>::value, "MSBFlag can only be used with integral types.");
        static_assert(std::is_unsigned<T>::value, "MSBFlag requires an unsigned integral type.");

        /// Default constructor initializes value and flag to zero.
        constexpr MSBFlag() noexcept;

        /// Constructor that initializes with a value and a flag.
        /// @param value The value to store.
        /// @param flag The flag to store (true or false).
        constexpr MSBFlag(T value, bool flag) noexcept;

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
        constexpr bool operator==(const MSBFlag& other) const noexcept;

        /// Inequality operator.
        constexpr bool operator!=(const MSBFlag& other) const noexcept;

    private:
        T data;
        static constexpr T flagMask = static_cast<T>(1) << (sizeof(T) * 8 - 1);// MSB mask
    };

    // Implementation of MSBFlag

    /// Default constructor initializes data to zero.
    template<typename T>
    inline constexpr MSBFlag<T>::MSBFlag() noexcept
        : data(0)
    {
    }

    /// Constructor that initializes with a value and a flag.
    template<typename T>
    inline constexpr MSBFlag<T>::MSBFlag(T value, bool flag) noexcept
        : data((value & (flagMask - 1)) | (static_cast<T>(flag) ? flagMask : 0))
    {
        // Ensure that the value does not overflow when the MSB is reserved for the flag
        // The maximum storable value is (max of T) >> 1
        // Consider adding runtime checks if necessary
    }

    /// Sets the value, preserving the flag.
    template<typename T>
    inline void MSBFlag<T>::SetValue(T value) noexcept
    {
        data = (value & (flagMask - 1)) | (data & flagMask);
    }

    /// Retrieves the value by masking out the MSB.
    template<typename T>
    inline constexpr T MSBFlag<T>::GetValue() const noexcept
    {
        return data & (~flagMask);
    }

    /// Sets the flag, preserving the value.
    template<typename T>
    inline void MSBFlag<T>::SetFlag(bool flag) noexcept
    {
        if (flag)
        {
            data |= flagMask;
        }
        else
        {
            data &= ~flagMask;
        }
    }

    /// Retrieves the flag by checking the MSB.
    template<typename T>
    inline constexpr bool MSBFlag<T>::GetFlag() const noexcept
    {
        return (data & flagMask) != 0;
    }

    /// Sets both the value and the flag.
    template<typename T>
    inline void MSBFlag<T>::Set(T value, bool flag) noexcept
    {
        data = (value & (flagMask - 1)) | (flag ? flagMask : 0);
    }

    /// Retrieves the raw data.
    template<typename T>
    inline constexpr T MSBFlag<T>::GetRaw() const noexcept
    {
        return data;
    }

    /// Sets the raw data directly.
    template<typename T>
    inline void MSBFlag<T>::SetRaw(T data) noexcept
    {
        this->data = data;
    }

    /// Equality operator compares the raw data.
    template<typename T>
    inline constexpr bool MSBFlag<T>::operator==(const MSBFlag& other) const noexcept
    {
        return data == other.data;
    }

    /// Inequality operator compares the raw data.
    template<typename T>
    inline constexpr bool MSBFlag<T>::operator!=(const MSBFlag& other) const noexcept
    {
        return data != other.data;
    }

}// namespace NGIN::Utilities
