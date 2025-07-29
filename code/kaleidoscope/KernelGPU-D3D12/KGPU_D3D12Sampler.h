//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:08:46
//

#pragma once

#include <KernelGPU/KGPU_Sampler.h>

#include "KGPU_D3D12BindlessManager.h"

namespace KGPU
{
    class D3D12Device;

    class D3D12Sampler : public ISampler
    {
    public:
        D3D12Sampler(D3D12Device* device, SamplerDesc desc);
        ~D3D12Sampler();

    private:
        D3D12Device* mParentDevice;

        D3D12BindlessAlloc mAlloc;
    };
}
