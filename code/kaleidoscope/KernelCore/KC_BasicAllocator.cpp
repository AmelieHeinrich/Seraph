//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:54:32
//

#include "KC_BasicAllocator.h"

namespace KC
{
    void* BasicAllocator::Allocate(uint64 size, int line, const char* file)
    {
        return KOS::AllocateMem(size);
    }

    void BasicAllocator::Free(void* block)
    {
        return KOS::FreeMem(block);
    }

    void BasicAllocator::Clear()
    {
        // ...
    }
}
