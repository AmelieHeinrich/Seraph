//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 14:00:14
//

#include "KC_TrackingAllocator.h"
#include "KC_Context.h"

namespace KC
{
    void* TrackingAllocator::Allocate(uint64 size, int line, const char* file)
    {
        MemoryInfo info = {};
        info.Line = line;
        info.Size = size;
        info.File = file;

        void* memory = KOS::AllocateMem(size);
        if (!memory) {
            KD_ERROR("Allocation failed for %llu bytes at %s:%d", size, file, line);
            return nullptr;
        }
        mTrackedAllocations.emplace(memory, std::move(info));
        return memory;
    }

    void TrackingAllocator::Free(void* block)
    {
        auto it = mTrackedAllocations.find(block);
        if (it != mTrackedAllocations.end()) {
            KOS::FreeMem(block);
            mTrackedAllocations.erase(it);
        } else {
            KD_ERROR("Double free or invalid pointer detected: %p", block);
        }
    }

    void TrackingAllocator::Clear()
    {
        for (auto& [ptr, _] : mTrackedAllocations) {
            KOS::FreeMem(ptr);
        }

        mTrackedAllocations.clear();
    }

    void TrackingAllocator::Report()
    {
        if (mTrackedAllocations.size() == 0) {
            printf("No leaks found. Good job!\n");
            return;
        }

        for (auto& [ptr, data] : mTrackedAllocations) {
            printf("Leak of %zu bytes at %s:%d\n", data.Size, data.File.c_str(), data.Line);
        }
    }
}
