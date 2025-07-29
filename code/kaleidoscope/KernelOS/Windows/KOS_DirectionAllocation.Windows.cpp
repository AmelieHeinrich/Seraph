//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 16:15:31
//

#include "KOS_Common.h"

#include <stdlib.h>
#include <stdio.h>

namespace KOS {
    static int allocCount = 0;

    void* AllocateMem(uint64 size)
    {
        allocCount++;
        return malloc(size);
    }

    void* AllocateMem(uint64 size, uint64 align)
    {
        return malloc(Align<uint64>(size, align));
    }

    void FreeMem(void* ptr)
    {
        allocCount--;
        if (ptr) free(ptr);
    }
}
