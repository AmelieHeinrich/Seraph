//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:05:21
//

#pragma once

#include <KernelGPU/KGPU_TLAS.h>

#include "KGPU_D3D12BindlessManager.h"

namespace KGPU
{
    class D3D12Device;

    class D3D12TLAS : public ITLAS
    {
    public:
        D3D12TLAS(D3D12Device* device);
        ~D3D12TLAS();
    
        uint64 Address() const { return mMemory->GetAddress(); }
    private:
        friend class D3D12CommandList;
    
        D3D12Device* mParentDevice;
    
        D3D12BindlessAlloc mAlloc;
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
    };
}
