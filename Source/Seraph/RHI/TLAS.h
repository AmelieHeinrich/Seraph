//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 21:53:39
//

#pragma once

#include <Core/Context.h>

#include "Bindless.h"

constexpr uint MAX_TLAS_INSTANCES = 8092;

struct TLASInstance
{
    float Transform[3][4];
    uint InstanceCustomIndex:24;
    uint Mask:8;
    uint InstanceShaderBindingTableRecordOffset:24;
    uint Flags:8;
    uint64 AccelerationStructureReference;
};

class IRHITLAS
{
public:
    ~IRHITLAS() = default;

    BindlessHandle GetBindlessHandle() const { return mBindless; }
protected:
    BindlessHandle mBindless;
};
