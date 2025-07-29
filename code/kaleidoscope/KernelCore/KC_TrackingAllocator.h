//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:58:37
//

#pragma once

#include <KernelOS/KOS_DirectAllocation.h>

#include "KC_Allocator.h"
#include "KC_String.h"
#include "KC_HashMap.h"

namespace KC
{
    class TrackingAllocator : public IMemoryAllocator
    {
    public:
        TrackingAllocator() = default;
        ~TrackingAllocator() = default;

        void* Allocate(uint64 size, int line = 0, const char* file = "") override;
        void Free(void* block) override;
        void Clear() override;
        void Report() override;
    private:
        struct MemoryInfo {
            uint64 Size;
            String File;
            int Line;
        };

        HashMap<void*, MemoryInfo> mTrackedAllocations;
    };
}
