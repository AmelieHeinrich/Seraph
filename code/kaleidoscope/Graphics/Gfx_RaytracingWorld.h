//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:36:44
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

#include "Gfx_Manager.h"
#include "Gfx_MeshPrimitive.h"

namespace Gfx
{
    class RaytracingWorld
    {
    public:
        RaytracingWorld();
        ~RaytracingWorld();

        void Reset();
        void AddInstance(Gfx::MeshPrimitive* primitive, KGPU::float4x4 transform, bool opaque = true);
        void Build(KGPU::ICommandList* list);
    
        KGPU::BindlessHandle GetWorldView() { return mTLAS->GetBindlessHandle(); }
    private:
        KGPU::ITLAS* mTLAS;
        KGPU::IBuffer* mInstanceBuffer;
        KC::Array<KGPU::TLASInstance> mInstances;
    };
}
