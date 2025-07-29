//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:11:16
//

#pragma once

#include "KGPU_ShaderCompat.h"
#include "KGPU_Bindless.h"
#include "KGPU_Buffer.h"

#include <glm/glm.hpp>

namespace KGPU
{
    constexpr uint MAX_TLAS_INSTANCES = 16000;
    constexpr uint TLAS_INSTANCE_OPAQUE = 0x00000004;
    constexpr uint TLAS_INSTANCE_NON_OPAQUE = 0x00000008;

    struct TLASInstance
    {
        float3x4 Transform;
        uint InstanceCustomIndex:24;
        uint Mask:8;
        uint InstanceShaderBindingTableRecordOffset:24;
        uint Flags:8;
        uint64 AccelerationStructureReference;
    };

    class ITLAS
    {
    public:
        virtual ~ITLAS() = default;

        BindlessHandle GetBindlessHandle() const { return mBindless; }

        IBuffer* GetMemory() { return mMemory; }
        IBuffer* GetScratch() { return mScratch; }
    protected:
        BindlessHandle mBindless;

        IBuffer* mMemory;
        IBuffer* mScratch;
    };
}
