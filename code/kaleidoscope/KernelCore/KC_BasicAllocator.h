//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:54:11
//

#pragma once

#include <KernelOS/KOS_DirectAllocation.h>

#include "KC_Allocator.h"

namespace KC
{
    class BasicAllocator : public IMemoryAllocator
    {
    public:
        BasicAllocator() = default;
        ~BasicAllocator() = default;

        void* Allocate(uint64 size, int line = 0, const char* file = "") override;
        void Free(void* block) override;
        void Clear() override;
    };
}
