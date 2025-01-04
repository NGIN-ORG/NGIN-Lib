/// @file TypeTraits.hpp
/// @brief Provides compile-time reflection utilities for extracting type names at compile time.
/// @note This example is compiler-dependent and works best with Clang, GCC, or MSVC in C++17+ or higher.

#pragma once

#include <array>
#include <string_view>
#include <type_traits>

namespace NGIN::Meta
{
    /// @brief A helper to detect if T is a template specialization
    /// @tparam T The type to detect
    template<typename T>
    struct IsTemplate : std::false_type {};

    /// @brief Partial specialization for any template<class...>
    /// @tparam Template The template template parameter
    /// @tparam Args     The template parameter pack
    template <template <typename...> class Template, typename... Args>
    struct IsTemplate<Template<Args...>> : std::true_type {};

    /// @brief A convenience boolean for IsTemplate<T>::value
    template <typename T>
    inline constexpr bool IsTemplateV = IsTemplate<T>::value;

    //------------------------------------------------------------------------------
    //  Compile-time string manipulation utilities
    //------------------------------------------------------------------------------

    /// @brief A simple compile-time "min" function
    /// @param a First argument
    /// @param b Second argument
    /// @return The lesser of a and b
    constexpr std::size_t ConstexprMin(std::size_t a, std::size_t b)
    {
        return (a < b) ? a : b;
    }

    /// @brief A naive compile-time strlen that stops at N or null terminator
    /// @param str The input string pointer
    /// @param maxLen The maximum number of characters to count
    /// @return The length found
    constexpr std::size_t ConstexprStrnlen(const char* str, std::size_t maxLen)
    {
        std::size_t i = 0;
        while (i < maxLen && str[i] != '\0')
        {
            ++i;
        }
        return i;
    }

    /// @brief Removes "class ", "struct ", "enum ", "union " from the input string.
    ///        Writes into outBuffer until outBufferSize-1 or end of input.
    /// @param input The input string to parse
    /// @param outBuffer The output buffer
    /// @param outBufferSize The size of outBuffer
    /// @return The number of characters actually written (excluding null terminator)
    constexpr std::size_t RemoveClassStructEnumTokens(
        std::string_view input,
        char* outBuffer,
        std::size_t outBufferSize)
    {
        constexpr std::array<const char*, 4> tokens = {"class ", "struct ", "enum ", "union "};
        std::size_t outPos = 0;
        std::size_t i = 0;

        while (i < input.size() && outPos + 1 < outBufferSize)
        {
            bool foundToken = false;
            for (auto&& tk : tokens)
            {
                const std::size_t tkLen = ConstexprStrnlen(tk, 32);
                if (i + tkLen <= input.size() && (input.compare(i, tkLen, tk) == 0))
                {
                    i += tkLen; // skip token
                    foundToken = true;
                    break;
                }
            }

            if (!foundToken)
            {
                // Copy one char
                outBuffer[outPos++] = input[i++];
            }
        }

        // Null-terminate
        if (outPos < outBufferSize)
        {
            outBuffer[outPos] = '\0';
        }
        return outPos;
    }

    /// @brief A helper function to trim trailing spaces in-place (at compile time).
    /// @param buffer The buffer containing the string
    /// @param len The length of the string
    constexpr void RTrim(char* buffer, std::size_t& len)
    {
        while (len > 0 && (buffer[len - 1] == ' ' || buffer[len - 1] == '\t'))
        {
            buffer[len - 1] = '\0';
            --len;
        }
    }

    //------------------------------------------------------------------------------
    //  Finds the last "top-level" "::" in a type string,
    //  ignoring any "::" that appear inside template arguments (angle brackets).
    //------------------------------------------------------------------------------
    constexpr std::size_t FindLastTopLevelDoubleColon(std::string_view s)
    {
        int bracketDepth = 0; // how many nested template brackets we are in
        // We'll search from the end to the beginning
        for (std::size_t i = s.size(); i > 1; /* decrement in loop */)
        {
            --i;
            // If we see '>', we increase bracketDepth => we are "deeper" in template args
            if (s[i] == '>')
            {
                ++bracketDepth;
            }
            // If we see '<', we decrease bracketDepth => we are leaving template args
            else if (s[i] == '<')
            {
                --bracketDepth;
            }
            // If bracketDepth == 0, we are at top-level
            // Check if this is "::"
            else if (s[i] == ':' && s[i - 1] == ':' && bracketDepth == 0)
            {
                return i - 1; // return the index of the first colon in "::"
            }
        }
        return std::string_view::npos;
    }

    //------------------------------------------------------------------------------
    //  Extracting the raw type name at compile time
    //------------------------------------------------------------------------------

    /// @brief Returns the raw type name from the compiler's function signature
    /// @tparam T The type to inspect
    /// @note This string still includes any "class ", "struct ", etc. We'll remove those next.
    template <typename T>
    constexpr std::string_view GetRawTypeName()
    {
    #if defined(__clang__) || defined(__GNUC__)
        // Example from Clang / GCC:
        // "constexpr std::string_view NGIN::Meta::GetRawTypeName() [T = int]"
        constexpr std::string_view signature = __PRETTY_FUNCTION__;
        constexpr std::string_view prefix    = "GetRawTypeName() [with T = ";
        constexpr std::string_view suffix    = "]";
    #elif defined(_MSC_VER)
        // Example from MSVC:
        // "constexpr std::string_view __cdecl NGIN::Meta::GetRawTypeName<int>(void)"
        constexpr std::string_view signature = __FUNCSIG__;
        constexpr std::string_view prefix    = "GetRawTypeName<";
        constexpr std::string_view suffix    = ">(void)";
    #else
    #error Unsupported compiler
    #endif
        const std::size_t startPos = signature.find(prefix);
        if (startPos == std::string_view::npos)
        {
            return "Unknown";
        }
        const std::size_t contentPos = startPos + prefix.size();
        const std::size_t endPos = signature.rfind(suffix);
        if (endPos == std::string_view::npos || endPos <= contentPos)
        {
            return "Unknown";
        }

        return signature.substr(contentPos, endPos - contentPos);
    }

    //--------------------------------------------------------------------------------
    //  TypeTraitsBase
    //  Provides additional compile-time booleans about the type (const, pointer, etc.)
    //--------------------------------------------------------------------------------
    template <typename T>
    struct TypeTraitsBase
    {
    private:
        // We'll remove references (to check const/volatile on the raw type),
        // but keep them around to check is_reference, etc. separately.
        // For pointer, we strip pointer and cv from T for certain checks.
        using NoRefT       = std::remove_reference_t<T>;
        using NoRefNoCVT   = std::remove_cv_t<NoRefT>;
        using NoPtrNoRefCV = std::remove_pointer_t<NoRefNoCVT>;

    public:
        /// @brief Whether the type is const (ignoring references)
        static constexpr bool isConst = std::is_const_v<NoRefT>;

        /// @brief Whether the type is volatile (ignoring references)
        static constexpr bool isVolatile = std::is_volatile_v<NoRefT>;

        /// @brief Whether the type is a pointer (after removing cv, but not references)
        static constexpr bool isPointer = std::is_pointer_v<NoRefNoCVT>;

        /// @brief Whether the type is a reference (either lvalue or rvalue)
        static constexpr bool isReference = std::is_reference_v<T>;

        /// @brief Whether the type is an lvalue reference
        static constexpr bool isLvalueReference = std::is_lvalue_reference_v<T>;

        /// @brief Whether the type is an rvalue reference
        static constexpr bool isRvalueReference = std::is_rvalue_reference_v<T>;

        /// @brief Whether the type is an array (ignoring cv)
        static constexpr bool isArray = std::is_array_v<NoRefNoCVT>;

        /// @brief Whether the (decayed) type is an enum
        static constexpr bool isEnum = std::is_enum_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is a class
        static constexpr bool isClass = std::is_class_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is a union
        static constexpr bool isUnion = std::is_union_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is an integral
        static constexpr bool isIntegral = std::is_integral_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is a floating point
        static constexpr bool isFloatingPoint = std::is_floating_point_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is arithmetic (integral or float)
        static constexpr bool isArithmetic = std::is_arithmetic_v<NoRefNoCVT>;

        /// @brief Whether the (decayed) type is fundamental (int, float, bool, etc.)
        static constexpr bool isFundamental = std::is_fundamental_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is signed
        static constexpr bool isSigned = std::is_signed_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is unsigned
        static constexpr bool isUnsigned = std::is_unsigned_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is trivially copyable
        static constexpr bool isTriviallyCopyable = std::is_trivially_copyable_v<NoPtrNoRefCV>;

        /// @brief Whether the (decayed) type is void
        static constexpr bool isVoid = std::is_void_v<NoPtrNoRefCV>;
    };

    //--------------------------------------------------------------------------------
    //  Primary TypeTraits template for non-template types
    //--------------------------------------------------------------------------------

    /// @brief Removes pointer/reference/CV qualifiers from T for name extraction
    template <typename T>
    using RemoveCVRefPtrT = std::remove_cv_t<
                              std::remove_reference_t<
                                T>>;

    /// @brief Primary TypeTraits template for non-template types
    /// @tparam T The type to inspect
    template <typename T>
    struct TypeTraits : TypeTraitsBase<T>
    {
    private:
        using BaseT = RemoveCVRefPtrT<T>;

        /// @brief Builds a static array of chars that represent the cleaned raw name.
        ///        This is done at compile time.
        static constexpr auto BuildRawNameArray()
        {
            constexpr std::string_view rawView = GetRawTypeName<BaseT>();
            std::array<char, 512> buf = {};

            // 1) remove "class ", "struct ", ...
            const std::size_t sizeWritten = RemoveClassStructEnumTokens(
                rawView,
                buf.data(),
                buf.size()
            );

            // 2) trim trailing spaces
            std::size_t len = sizeWritten;
            RTrim(buf.data(), len);

            return buf;
        }

        static constexpr auto rawNameArray_ = BuildRawNameArray();

        /// @brief Returns a string_view into rawNameArray_
        static constexpr std::string_view BuildRawName()
        {
            constexpr std::size_t len = ConstexprStrnlen(
                rawNameArray_.data(),
                rawNameArray_.size()
            );
            return std::string_view(rawNameArray_.data(), len);
        }

        /// @brief Build a bracket-aware "unqualified name"
        static constexpr std::string_view BuildUnqualifiedName(std::string_view qn)
        {
            const std::size_t pos = FindLastTopLevelDoubleColon(qn);
            if (pos != std::string_view::npos)
            {
                // skip "::"
                return qn.substr(pos + 2);
            }
            return qn;
        }

        /// @brief Build a bracket-aware "namespace"
        static constexpr std::string_view BuildNamespaceName(std::string_view qn, std::string_view unq)
        {
            // If unqualified name is the entire string, there's no namespace
            if (qn.size() == unq.size())
            {
                return {};
            }
            // Otherwise, the portion before the "::" is the namespace
            const std::size_t pos = qn.size() - unq.size();
            // We skip the "::", so we remove an additional 2
            if (pos >= 2)
            {
                return qn.substr(0, pos - 2);
            }
            return std::string_view{};
        }

    public:
        /// @brief The raw name as extracted from the signature and cleaned of class/struct
        static constexpr std::string_view rawName = BuildRawName();

        /// @brief For non-template types, "qualified name" == raw name
        static constexpr std::string_view qualifiedName = rawName;

        /// @brief Extract the unqualified name by ignoring any "::" inside <...> (rare for non-templates, but consistent).
        static constexpr std::string_view unqualifiedName = BuildUnqualifiedName(qualifiedName);

        /// @brief Extract the namespace portion by removing the unqualified from Qualified
        static constexpr std::string_view namespaceName = BuildNamespaceName(qualifiedName, unqualifiedName);
    };

    //--------------------------------------------------------------------------------
    //  Partial Specialization for Template Types
    //--------------------------------------------------------------------------------

/// @brief Partial Specialization for Template Types
/// @tparam Template The template template parameter
/// @tparam Args The template parameter pack
template <template <typename...> class Template, typename... Args>
struct TypeTraits<Template<Args...>> : TypeTraitsBase<Template<Args...>>
{
private:
    using ThisT = Template<Args...>;

    //------------------------------------------------------------------------
    //  1) RAW NAME (from compiler)
    //------------------------------------------------------------------------
    static constexpr auto BuildRawNameArray()
    {
        constexpr std::string_view rawView = GetRawTypeName<ThisT>();
        std::array<char, 512> buf = {};

        // remove "class ", "struct ", etc.
        const std::size_t sizeWritten = RemoveClassStructEnumTokens(
            rawView,
            buf.data(),
            buf.size()
        );

        // trim trailing spaces
        std::size_t len = sizeWritten;
        RTrim(buf.data(), len);

        return buf;
    }

    static constexpr auto rawNameArray_ = BuildRawNameArray();

    /// @brief e.g. "std::vector<int, std::allocator<int>>"
    static constexpr std::string_view BuildRawName()
    {
        constexpr std::size_t len = ConstexprStrnlen(
            rawNameArray_.data(),
            rawNameArray_.size()
        );
        return { rawNameArray_.data(), len };
    }

    //------------------------------------------------------------------------
    //  2) BASE TEMPLATE NAME (Qualified)
    //     We'll parse out everything before the first '<'.
    //------------------------------------------------------------------------
    static constexpr auto BuildQualifiedBaseArray()
    {
        std::array<char, 512> baseBuf = {};
        constexpr std::string_view fullRaw = BuildRawName();

        std::size_t i = 0;
        std::size_t j = 0;
        while (i < fullRaw.size() && fullRaw[i] != '<' && (j + 1) < baseBuf.size())
        {
            baseBuf[j++] = fullRaw[i++];
        }
        if (j < baseBuf.size())
        {
            baseBuf[j] = '\0';
        }
        return baseBuf;
    }

    static constexpr auto qualifiedBaseNameArray_ = BuildQualifiedBaseArray();

    /// @brief e.g. "std::vector"
    static constexpr std::string_view BuildQualifiedBaseName()
    {
        constexpr std::size_t len = ConstexprStrnlen(
            qualifiedBaseNameArray_.data(),
            qualifiedBaseNameArray_.size()
        );
        return { qualifiedBaseNameArray_.data(), len };
    }

    static constexpr std::string_view qualifiedBaseName = BuildQualifiedBaseName();

    //------------------------------------------------------------------------
    //  2.b) BASE TEMPLATE NAME (Unqualified)
    //     We bracket-aware search for the last top-level `::` in `qualifiedBaseName`.
    //------------------------------------------------------------------------
    static constexpr std::string_view BuildUnqualifiedBaseName()
    {
        constexpr std::string_view qb = qualifiedBaseName;
        // We'll do a bracket-aware search for the last top-level "::".
        // But note, there's no `<...>` in qb because we've stripped it out above.
        // So we can do a simpler non-bracket approach, or re-use the existing function:
        std::size_t pos = FindLastTopLevelDoubleColon(qb);
        if (pos == std::string_view::npos)
        {
            return qb; // entire base name is unqualified
        }
        return qb.substr(pos + 2); // skip "::"
    }

    static constexpr std::string_view unqualifiedBaseName = BuildUnqualifiedBaseName();

    //------------------------------------------------------------------------
    //  3) BUILD <Arg...> FOR QUALIFIED NAME
    //------------------------------------------------------------------------
    static constexpr std::size_t BuildQualifiedTemplateArgs(char* out, std::size_t maxLen)
    {
        if (maxLen < 2)
            return 0; // can't even put "<>"

        std::size_t pos = 0;
        out[pos++] = '<';

        bool first = true;
        (([&]
        {
            if (!first)
            {
                // comma + space
                if (pos + 2 < maxLen)
                {
                    out[pos++] = ',';
                    out[pos++] = ' ';
                }
            }
            else
            {
                first = false;
            }
            constexpr auto& argQN = TypeTraits<Args>::qualifiedName;
            for (std::size_t i = 0; i < argQN.size() && (pos + 1 < maxLen); ++i)
            {
                out[pos++] = argQN[i];
            }
        }()), ...);

        if (pos + 1 < maxLen)
        {
            out[pos++] = '>';
        }
        if (pos < maxLen)
        {
            out[pos] = '\0';
        }
        return pos;
    }

    //------------------------------------------------------------------------
    //  3.b) BUILD <Arg...> FOR UNQUALIFIED NAME
    //      e.g. "<int, allocator<int>>"
    //------------------------------------------------------------------------
    static constexpr std::size_t BuildUnqualifiedTemplateArgs(char* out, std::size_t maxLen)
    {
        if (maxLen < 2)
            return 0; // can't even put "<>"

        std::size_t pos = 0;
        out[pos++] = '<';

        bool first = true;
        (([&]
        {
            if (!first)
            {
                // comma + space
                if (pos + 2 < maxLen)
                {
                    out[pos++] = ',';
                    out[pos++] = ' ';
                }
            }
            else
            {
                first = false;
            }
            // use unqualifiedName of each argument
            constexpr auto& argUNQ = TypeTraits<Args>::unqualifiedName;
            for (std::size_t i = 0; i < argUNQ.size() && (pos + 1 < maxLen); ++i)
            {
                out[pos++] = argUNQ[i];
            }
        }()), ...);

        if (pos + 1 < maxLen)
        {
            out[pos++] = '>';
        }
        if (pos < maxLen)
        {
            out[pos] = '\0';
        }
        return pos;
    }

    //------------------------------------------------------------------------
    //  4) BUILD FULL QUALIFIED NAME
    //------------------------------------------------------------------------
    static constexpr auto BuildQualifiedNameArray()
    {
        std::array<char, 1024> buf = {};
        std::size_t pos = 0;

        // 1. Copy the qualified base
        for (std::size_t i = 0; 
             i < qualifiedBaseName.size() && (pos + 1 < buf.size());
             ++i)
        {
            buf[pos++] = qualifiedBaseName[i];
        }

        // 2. Build <Arg...> with qualified
        if constexpr (sizeof...(Args) > 0)
        {
            pos += BuildQualifiedTemplateArgs(buf.data() + pos, buf.size() - pos);
        }

        if (pos < buf.size())
        {
            buf[pos] = '\0';
        }
        return buf;
    }

    static constexpr auto qualifiedNameArray_ = BuildQualifiedNameArray();

    static constexpr std::string_view BuildQualifiedName()
    {
        constexpr std::size_t len = ConstexprStrnlen(
            qualifiedNameArray_.data(),
            qualifiedNameArray_.size()
        );
        return { qualifiedNameArray_.data(), len };
    }

    //------------------------------------------------------------------------
    //  4.b) BUILD FULL UNQUALIFIED NAME
    //------------------------------------------------------------------------
    static constexpr auto BuildUnqualifiedNameArray()
    {
        std::array<char, 1024> buf = {};
        std::size_t pos = 0;

        // 1. Copy the unqualified base
        for (std::size_t i = 0;
             i < unqualifiedBaseName.size() && (pos + 1 < buf.size());
             ++i)
        {
            buf[pos++] = unqualifiedBaseName[i];
        }

        // 2. Build <Arg...> with unqualified
        if constexpr (sizeof...(Args) > 0)
        {
            pos += BuildUnqualifiedTemplateArgs(buf.data() + pos, buf.size() - pos);
        }

        if (pos < buf.size())
        {
            buf[pos] = '\0';
        }
        return buf;
    }

    static constexpr auto unqualifiedNameArray_ = BuildUnqualifiedNameArray();

    static constexpr std::string_view BuildUnqualifiedName()
    {
        constexpr std::size_t len = ConstexprStrnlen(
            unqualifiedNameArray_.data(),
            unqualifiedNameArray_.size()
        );
        return { unqualifiedNameArray_.data(), len };
    }

    //------------------------------------------------------------------------
    //  5) NAMESPACE (via bracket-aware search in the qualified name)
    //------------------------------------------------------------------------
    static constexpr std::size_t FindColonPos(std::string_view s)
    {
        return FindLastTopLevelDoubleColon(s);
    }

    static constexpr std::string_view BuildNamespaceFromQualified(std::string_view full)
    {
        const std::size_t pos = FindColonPos(full);
        if (pos == std::string_view::npos)
        {
            return {}; // no namespace
        }
        return full.substr(0, pos); // everything before the "::"
    }

public:
    /// @brief e.g. "std::vector<int, std::allocator<int>>"
    static constexpr std::string_view rawName = BuildRawName();

    /// @brief The fully qualified name, e.g. "std::vector<int, std::allocator<int>>"
    static constexpr std::string_view qualifiedName = BuildQualifiedName();

    /// @brief The fully unqualified name, e.g. "vector<int, allocator<int>>"
    static constexpr std::string_view unqualifiedName = BuildUnqualifiedName();

    /// @brief bracket-aware extraction from the full qualified name for the top-level namespace
    static constexpr std::string_view namespaceName =
        BuildNamespaceFromQualified(qualifiedName);
};

} // namespace NGIN::Meta
