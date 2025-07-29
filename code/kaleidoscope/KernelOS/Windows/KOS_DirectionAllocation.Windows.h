//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 15:53:08
//

#pragma once

#include "KOS_Common.h"

namespace KOS
{
    void* AllocateMem(uint64 size);
    void* AllocateMem(uint64 size, uint64 align);
    void FreeMem(void* ptr);
}
