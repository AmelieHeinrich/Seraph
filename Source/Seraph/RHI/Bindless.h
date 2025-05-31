//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 17:07:29
//

#pragma once

#include <Core/Context.h>

constexpr uint INVALID_HANDLE = 0xFFFFFF;

struct BindlessHandle
{
    uint Index = INVALID_HANDLE;

    BindlessHandle() = default;
    BindlessHandle(uint index)
        : Index(index) {}
};
