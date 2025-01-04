/// @file String.hpp
/// @brief Custom String class with small buffer optimization (SBO).

#pragma once

#include <cstring>
#include <utility>
#include <NGIN/Utilities/LSBFlag.hpp>
#include <iostream>

namespace NGIN::Containers
{
    /// @class String
    /// @brief A custom string class optimized for 3D application usage.
    ///
    /// This class utilizes Small Buffer Optimization (SBO) and bit manipulation to store small strings without heap allocation.
    /// It automatically switches to dynamic allocation for larger strings and uses the least significant bit to indicate the storage type.
    class String
    {
    public:
        /// @brief Default constructor. Creates an empty string in SBO mode.
        String();

        /// @brief Constructs a String from a C-style string.
        /// @param str The C-style string to initialize from (can be null).
        String(const char* str);

        /// @brief Copy constructor.
        /// @param other The String to copy from.
        String(const String& other);

        /// @brief Move constructor.
        /// @param other The String to move from.
        String(String&& other) noexcept;

        /// @brief Destructor. Frees any heap-allocated data if necessary.
        ~String();

        /// @brief Copy assignment operator.
        /// @param other The String to copy from.
        /// @return Reference to *this.
        String& operator=(const String& other);

        /// @brief Move assignment operator.
        /// @param other The String to move from.
        /// @return Reference to *this.
        String& operator=(String&& other) noexcept;

        /// @brief Append operator.
        /// @param other The String to append to *this.
        /// @return Reference to *this.
        String& operator+=(const String& other);

        /// @brief Append operator.
        /// @param other The CStr to append to *this.
        /// @return Reference to *this.
        String& operator+=(const char* const other);

        /// @brief Returns the length of the string (number of characters).
        /// @return Length of the string.
        std::size_t GetSize() const;

        /// @brief Returns a C-style null-terminated string.
        /// @return Pointer to the C-style string.
        const char* CStr() const;

        /// @brief Appends another String to this String.
        /// @param other The string to append.
        void Append(const String& other);

        /// @brief Appends a C-style string to this String.
        /// @param str The C-style string to append.
        void Append(const char* const str);

    private:
        /// @brief Small Buffer Optimization threshold.
        static constexpr std::size_t sboSize = 48;

        /// @struct NormalStorage
        /// @brief Structure to store heap data.
        struct NormalStorage
        {
            char* data;            ///< Pointer to the string data.
            std::size_t capacity;  ///< Capacity of the allocated data buffer.
            //Padding to make Sizeof(NormalStorage) == SboSize
            std::byte padding[sboSize - (sizeof(char*) + (sizeof(std::size_t) * 2))];
            /// We store size in `sizeFlag` using LSBFlag for the 'heap-vs-SBO' bit.
            /// The rest of the bits store the actual size.
            NGIN::Utilities::LSBFlag<std::size_t> sizeFlag;
        };

        /// @struct SmallStorage
        /// @brief Structure for SBO (small buffer optimization).
        struct SmallStorage
        {
            char sboBuffer[sboSize - 1]; ///< Buffer for SBO
            NGIN::Utilities::LSBFlag<uint8_t> remainingSizeFlag; ///< Byte that stores "remaining" + the SBO/heap flag
        };

        /// @union StorageUnion
        /// @brief Union for combining normal and SBO storage.
        union StorageUnion
        {
            NormalStorage normal;
            SmallStorage small;
            char data[sboSize]; // for easy copying with std::memcpy
        };

        /// @brief The underlying storage buffer (either small or normal).
        alignas(16) StorageUnion buffer{};

    private:
        /// @brief Checks if we are in SBO mode.
        /// @return True if SBO; false otherwise.
        bool IsSmall() const;

        /// @brief Allocates new heap data of the requested capacity and copies ewxisting contents into it.
        /// @param newCapacity Capacity for the newly allocated buffer.
        /// @param currentSize The current size of the string.
        void ReallocateAndCopy(std::size_t newCapacity, std::size_t currentSize);
    };

    //------------------------------------------------------------------------------
    //                               Implementation
    //------------------------------------------------------------------------------

    inline bool String::IsSmall() const
    {
        // If the LSB in remainingSizeFlag or sizeFlag is set => SBO
        return buffer.small.remainingSizeFlag.GetFlag();
    }

    inline std::size_t String::GetSize() const
    {
        if (IsSmall())
        {
            // The used portion in SBO is (sboSize - 1) - remaining
            std::size_t remaining = buffer.small.remainingSizeFlag.GetValue();
            return (sboSize - 1) - remaining;
        }
        else
        {
            return buffer.normal.sizeFlag.GetValue();
        }
    }

    inline const char* String::CStr() const
    {
        return IsSmall() ? buffer.small.sboBuffer : buffer.normal.data;
    }

    inline String::String()
    {
        /// IMPORTANT: zero out the union to avoid uninitialized bits.
        std::memset(&buffer, 0, sizeof(buffer));

        // Initialize as an empty SBO string
        uint8_t maxRemaining = static_cast<uint8_t>(sboSize - 1);
        buffer.small.remainingSizeFlag.Set(maxRemaining, true); 
        buffer.small.sboBuffer[0] = '\0';
    }

    inline String::String(const char* str)
    {
        std::memset(&buffer, 0, sizeof(buffer)); // <--- Zero out the union

        if (!str)
        {
            // Handle nullptr => empty
            uint8_t maxRemaining = static_cast<uint8_t>(sboSize - 1);
            buffer.small.remainingSizeFlag.Set(maxRemaining, true);
            buffer.small.sboBuffer[0] = '\0';
            return;
        }

        std::size_t len = std::strlen(str);
        if (len < (sboSize - 1))
        {
            // Fits in SBO
            std::size_t remaining = (sboSize - 1) - len;
            buffer.small.remainingSizeFlag.Set(static_cast<uint8_t>(remaining), true);
            std::memcpy(buffer.small.sboBuffer, str, len + 1);
        }
        else
        {
            // Use heap
            buffer.normal.sizeFlag.Set(len, false);  // Flag = false => heap
            buffer.normal.capacity = len + 1;
            buffer.normal.data     = new char[buffer.normal.capacity];
            std::memcpy(buffer.normal.data, str, len + 1);
        }
    }

    inline String::String(const String& other)
    {
        std::memset(&buffer, 0, sizeof(buffer)); // <--- Zero out the union

        if (other.IsSmall())
        {
            // Copy all 48 bytes of union data
            std::memcpy(buffer.data, other.buffer.data, sboSize);
        }
        else
        {
            // Deep copy the heap data
            std::size_t len = other.GetSize();
            buffer.normal.sizeFlag.Set(len, false);
            buffer.normal.capacity = other.buffer.normal.capacity;
            buffer.normal.data = new char[buffer.normal.capacity];
            std::memcpy(buffer.normal.data, other.buffer.normal.data, len + 1);
        }
    }

    inline String::String(String&& other) noexcept
    {
        std::memset(&buffer, 0, sizeof(buffer)); // <--- Zero out the union

        if (other.IsSmall())
        {
            // Copy the SBO portion
            std::memcpy(buffer.data, other.buffer.data, sboSize);
        }
        else
        {
            // Move the heap pointer
            std::size_t len = other.GetSize();
            buffer.normal.sizeFlag.Set(len, false);
            buffer.normal.capacity = other.buffer.normal.capacity;
            buffer.normal.data     = other.buffer.normal.data;

            // Nullify the other
            other.buffer.normal.sizeFlag.Set(0, false);
            other.buffer.normal.capacity = 0;
            other.buffer.normal.data = nullptr;
        }
    }

    inline String::~String()
    {
        if (!IsSmall() && buffer.normal.data != nullptr)
        {
            delete[] buffer.normal.data;
            buffer.normal.data = nullptr;
        }
    }

    inline String& String::operator=(const String& other)
    {
        if (this == &other)
        {
            return *this;
        }

        // Clean up any old heap data if we were big
        if (!IsSmall() && buffer.normal.data)
        {
            delete[] buffer.normal.data;
            buffer.normal.data = nullptr;
        }

        // Re-zero the union to avoid stale bits
        std::memset(&buffer, 0, sizeof(buffer));

        if (other.IsSmall())
        {
            // Copy entire SBO union
            std::memcpy(buffer.data, other.buffer.data, sboSize);
        }
        else
        {
            std::size_t len = other.GetSize();
            buffer.normal.sizeFlag.Set(len, false);
            buffer.normal.capacity = other.buffer.normal.capacity;
            buffer.normal.data = new char[buffer.normal.capacity];
            std::memcpy(buffer.normal.data, other.buffer.normal.data, len + 1);
        }

        return *this;
    }

    inline String& String::operator=(String&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        // Clean up any old heap data
        if (!IsSmall() && buffer.normal.data)
        {
            delete[] buffer.normal.data;
            buffer.normal.data = nullptr;
        }

        // Re-zero the union to avoid stale bits
        std::memset(&buffer, 0, sizeof(buffer));

        if (other.IsSmall())
        {
            // Copy the SBO memory
            std::memcpy(buffer.data, other.buffer.data, sboSize);
        }
        else
        {
            // Move the heap pointer
            std::size_t len = other.GetSize();
            buffer.normal.sizeFlag.Set(len, false);
            buffer.normal.capacity = other.buffer.normal.capacity;
            buffer.normal.data = other.buffer.normal.data;

            // Nullify the other
            other.buffer.normal.sizeFlag.Set(0, false);
            other.buffer.normal.capacity = 0;
            other.buffer.normal.data = nullptr;
        }
        return *this;
    }

    inline String& String::operator+=(const String& other)
    {
        Append(other);
        return *this;
    }

    inline String& String::operator+=(const char* const str)
    {
        Append(str);
        return *this;
    }

    inline void String::ReallocateAndCopy(std::size_t newCapacity, std::size_t currentSize)
    {
        char* newData = new char[newCapacity];

        if (IsSmall())
        {
            // Copy from SBO
            std::memcpy(newData, buffer.small.sboBuffer, currentSize);
        }
        else
        {
            // Copy from existing heap data
            std::memcpy(newData, buffer.normal.data, currentSize);
            delete[] buffer.normal.data;
        }

        buffer.normal.sizeFlag.Set(currentSize, false);
        buffer.normal.capacity = newCapacity;
        buffer.normal.data     = newData;
    }

    inline void String::Append(const String& other)
    {
        std::size_t currentSize = GetSize();
        std::size_t otherSize   = other.GetSize();
        std::size_t totalSize   = currentSize + otherSize;

        if (IsSmall())
        {
            std::size_t remaining = buffer.small.remainingSizeFlag.GetValue();
            if (remaining > otherSize)
            {
                // Still fits in SBO
                std::memcpy(buffer.small.sboBuffer + currentSize,
                            other.CStr(),
                            otherSize + 1);
                buffer.small.remainingSizeFlag.Set(
                    static_cast<uint8_t>(remaining - otherSize),
                    true // remain in SBO mode
                );
            }
            else
            {
                // Switch to heap
                std::size_t newCapacity = (totalSize * 2) + 1;
                ReallocateAndCopy(newCapacity, currentSize);
                std::memcpy(buffer.normal.data + currentSize, other.CStr(), otherSize + 1);
            }
        }
        else
        {
            // Already on heap
            if (totalSize + 1 > buffer.normal.capacity)
            {
                std::size_t newCapacity = (totalSize * 2) + 1;
                ReallocateAndCopy(newCapacity, currentSize);
            }
            std::memcpy(buffer.normal.data + currentSize, other.CStr(), otherSize + 1);
        }

        // If we are on the heap, update the size
        if (!IsSmall())
        {
            buffer.normal.sizeFlag.Set(totalSize, false);
        }
    }

inline void String::Append(const char* const str)
{
    if (!str)
    {
        // Treat nullptr as empty string; nothing to append
        return;
    }

    std::size_t appendSize = std::strlen(str);
    if (appendSize == 0)
    {
        // Nothing to append
        return;
    }

    std::size_t currentSize = GetSize();
    std::size_t totalSize = currentSize + appendSize;

    if (IsSmall())
    {
        std::size_t remaining = buffer.small.remainingSizeFlag.GetValue();

        if (remaining > appendSize)
        {
            // Still fits in SBO
            std::memcpy(buffer.small.sboBuffer + currentSize, str, appendSize + 1); // +1 to copy the null terminator
            buffer.small.remainingSizeFlag.Set(
                static_cast<uint8_t>(remaining - appendSize),
                true // remain in SBO mode
            );
        }
        else
        {
            // Switch to heap
            std::size_t newCapacity = (totalSize * 2) + 1; // Double the required size for future growth
            ReallocateAndCopy(newCapacity, currentSize);
            std::memcpy(buffer.normal.data + currentSize, str, appendSize + 1); // +1 to copy the null terminator
        }
    }
    else
    {
        // Already on heap
        if (totalSize + 1 > buffer.normal.capacity)
        {
            // Need to reallocate with increased capacity
            std::size_t newCapacity = (totalSize * 2) + 1; // Double the required size for future growth
            ReallocateAndCopy(newCapacity, currentSize);
        }
        std::memcpy(buffer.normal.data + currentSize, str, appendSize + 1); // +1 to copy the null terminator
    }

    // Update the size flags if we're on the heap
    if (!IsSmall())
    {
        buffer.normal.sizeFlag.Set(totalSize, false);
    }
}


} // namespace NGIN::Containers
