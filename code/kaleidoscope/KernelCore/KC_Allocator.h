//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:47:47
//

#pragma once

#include <KernelOS/KOS_Common.h>

namespace KC
{
    class IMemoryAllocator
    {
    public:
        virtual ~IMemoryAllocator() = default;

        virtual void* Allocate(uint64 size, int line = 0, const char* file = "") = 0;
        virtual void Free(void* block) = 0;
        virtual void Clear() = 0;
        virtual void Report() {}
    };
}
