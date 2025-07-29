//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:09:19
//

#include "KGPU_D3D12Sampler.h"
#include "KGPU_D3D12Device.h"

namespace KGPU
{
    D3D12Sampler::D3D12Sampler(D3D12Device* device, SamplerDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        mAlloc = mParentDevice->GetBindlessManager()->WriteSampler(this);
        mHandle.Index = mAlloc.Index;

        KD_WHATEVER("Created D3D12 sampler!");
    }

    D3D12Sampler::~D3D12Sampler()
    {
        mParentDevice->GetBindlessManager()->FreeSampler(mAlloc);
    }
}
