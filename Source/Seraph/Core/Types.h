//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 06:48:22
//

#pragma once

#include <cstdint>

#include <string>
#include <queue>
#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include <unordered_map>

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using uint = uint32_t;

static_assert(sizeof(int8) == 1, "int8 size incorrect.");
static_assert(sizeof(int16) == 2, "int16 size incorrect.");
static_assert(sizeof(int32) == 4, "int32 size incorrect.");
static_assert(sizeof(int64) == 8, "int64 size incorrect.");
static_assert(sizeof(uint8) == 1, "uint8 size incorrect.");
static_assert(sizeof(uint16) == 2, "uint16 size incorrect.");
static_assert(sizeof(uint32) == 4, "uint32 size incorrect.");
static_assert(sizeof(uint64) == 8, "uint64 size incorrect.");

using String = std::string;
using StringView = std::string_view;

template<typename T>
using Array = std::vector<T>;

template<typename T, size_t Size>
using StaticArray = std::array<T, Size>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename K, typename T>
using UnorderedMap = std::unordered_map<K, T>;

template<typename F, typename S>
using Pair = std::pair<F, S>;

template<typename T>
constexpr T Align(T s, T a)
{
    return ((s + a - 1) & ~(a - 1));
}

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

void SafeMemcpy(void* dst, const void* src, uint64 size);
