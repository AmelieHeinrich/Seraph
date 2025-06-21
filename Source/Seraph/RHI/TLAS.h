//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 21:53:39
//

#pragma once

#include <Core/Context.h>
#include <glm/glm.hpp>

#include "Bindless.h"
#include "Buffer.h"

constexpr uint MAX_TLAS_INSTANCES = 16000;
constexpr uint TLAS_INSTANCE_OPAQUE = 0x00000004;
constexpr uint TLAS_INSTANCE_NON_OPAQUE = 0x00000008;

struct TLASInstance
{
    glm::mat3x4 Transform;
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

    IRHIBuffer* GetMemory() { return mMemory; }
    IRHIBuffer* GetScratch() { return mScratch; }
protected:
    BindlessHandle mBindless;

    IRHIBuffer* mMemory;
    IRHIBuffer* mScratch;
};
