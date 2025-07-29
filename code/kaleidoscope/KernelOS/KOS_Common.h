//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 13:42:42
//

#pragma once

#include <cstdint>
#include <type_traits>

#include "KOS_Main.h"

#define CODE_BLOCK(message)

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using uint = uint32;

static_assert(sizeof(int8) == 1, "int8 size incorrect.");
static_assert(sizeof(int16) == 2, "int16 size incorrect.");
static_assert(sizeof(int32) == 4, "int32 size incorrect.");
static_assert(sizeof(int64) == 8, "int64 size incorrect.");
static_assert(sizeof(uint8) == 1, "uint8 size incorrect.");
static_assert(sizeof(uint16) == 2, "uint16 size incorrect.");
static_assert(sizeof(uint32) == 4, "uint32 size incorrect.");
static_assert(sizeof(uint64) == 8, "uint64 size incorrect.");

#define BIT(x) (1 << x)
#define KILOBYTES(s) s * 1024
#define MEGABYTES(s) KILOBYTES(s) * 1024
#define GIGABYTES(s) MEGABYTES(s) * 1024

#define ENUM_CLASS_FLAGS(EnumType)                                                \
inline constexpr EnumType operator|(EnumType lhs, EnumType rhs) {                 \
    using T = std::underlying_type_t<EnumType>;                                   \
    return static_cast<EnumType>(static_cast<T>(lhs) | static_cast<T>(rhs));      \
}                                                                                 \
inline constexpr EnumType& operator|=(EnumType& lhs, EnumType rhs) {              \
    lhs = lhs | rhs;                                                              \
    return lhs;                                                                   \
}                                                                                 \
inline constexpr EnumType operator&(EnumType lhs, EnumType rhs) {                 \
    using T = std::underlying_type_t<EnumType>;                                   \
    return static_cast<EnumType>(static_cast<T>(lhs) & static_cast<T>(rhs));      \
}                                                                                 \
inline constexpr EnumType& operator&=(EnumType& lhs, EnumType rhs) {              \
    lhs = lhs & rhs;                                                              \
    return lhs;                                                                   \
}                                                                                 \
inline constexpr bool Any(EnumType value) {                                       \
    using T = std::underlying_type_t<EnumType>;                                   \
    return static_cast<T>(value) != 0;                                            \
}

namespace KOS {
    template<typename T>
    constexpr T Align(T s, T a)
    {
        return ((s + a - 1) & ~(a - 1));
    }
}
