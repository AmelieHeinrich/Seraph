//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:02:44
//

#pragma once

#include <RHI/Sampler.h>

#include <Agility/d3d12.h>

#include "D3D12BindlessManager.h"

class D3D12Device;

class D3D12Sampler : public IRHISampler
{
public:
    D3D12Sampler(D3D12Device* device, RHISamplerDesc desc);
    ~D3D12Sampler();

private:
    D3D12Device* mParentDevice;

    D3D12BindlessAlloc mAlloc;
};
