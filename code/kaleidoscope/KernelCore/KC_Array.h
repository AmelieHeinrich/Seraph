//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 16:36:10
//

#pragma once

#include "KC_Common.h"
#include <KernelOS/KOS_DirectAllocation.h>

#include <vector>
#include <array>
#include <set>

namespace KC {
    template<typename T>
    using Array = std::vector<T>;

    template<typename T, int N>
    using StaticArray = std::array<T, N>;

    template<typename T>
    using Set = std::set<T>;
}
