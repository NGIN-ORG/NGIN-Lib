#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#define NGIN_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define NGIN_ALWAYS_INLINE inline __attribute__((always_inline))
#else
#define NGIN_ALWAYS_INLINE inline
#endif