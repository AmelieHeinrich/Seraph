//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:00:51
//


#pragma once

#include <KernelCore/KC_Context.h>

namespace KGPU
{
    constexpr uint BINDLESS_INVALID_HANDLE = 0x00000000;

    struct BindlessHandle
    {
        uint Index = BINDLESS_INVALID_HANDLE;

        BindlessHandle() = default;
        BindlessHandle(uint index)
            : Index(index) {}
    };
}
