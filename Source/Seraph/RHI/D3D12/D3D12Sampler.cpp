//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:03:15
//

#include "D3D12Sampler.h"
#include "D3D12Device.h"

D3D12Sampler::D3D12Sampler(D3D12Device* device, RHISamplerDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    mAlloc = mParentDevice->GetBindlessManager()->WriteSampler(this);
    mHandle.Index = mAlloc.Index;

    SERAPH_WHATEVER("Created D3D12 sampler!");
}

D3D12Sampler::~D3D12Sampler()
{
    mParentDevice->GetBindlessManager()->FreeSampler(mAlloc);
}
