//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:21:35
//

#pragma once

#include "KC_String.h"

namespace KC
{
    template<typename T>
    inline size_t HashCombine(size_t seed, const T& v)
    {
        return seed ^ (std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    size_t Hash(const KC::String& str);
}
