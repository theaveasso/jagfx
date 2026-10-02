#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h> // JA_ASSERT prints with fprintf

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef int64_t   s64;
typedef float     f32;
typedef double    f64;
typedef int32_t   b32;
typedef uintptr_t umm;

static_assert(sizeof(umm) == sizeof(void *));
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

template <typename T, size_t N>
constexpr size_t
ArrayCount(T (&Array)[N])
{
    (void)Array;
    return N;
}

#if defined(_MSC_VER)
#define JA_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#define JA_DEBUGBREAK() __builtin_trap()
#else
#define JA_DEBUGBREAK() abort()
#endif

#if !defined(NDEBUG)
#define JA_ASSERT(Expr)                                                       \
    do                                                                        \
    {                                                                         \
        if(!(Expr))                                                           \
        {                                                                     \
            fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #Expr); \
            JA_DEBUGBREAK();                                                  \
        }                                                                     \
    } while(0)
#else
#define JA_ASSERT(Expr) ((void)sizeof(Expr))
#endif

constexpr size_t
KB(int N)
{
    return (1ull << 10) * N;
}

constexpr size_t
MB(int N)
{
    return (1ull << 20) * N;
}

constexpr size_t
GB(int N)
{
    return (1ull << 30) * N;
}

inline constexpr u32
PackColor(u8 R, u8 G, u8 B, u8 A = 255)
{
    return u32(R) | (u32(G) << 8) | (u32(B) << 16) | (u32(A) << 24);
}
