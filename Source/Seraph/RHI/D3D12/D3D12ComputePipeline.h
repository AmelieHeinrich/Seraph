//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:04:31
//

#pragma once

#include <RHI/ComputePipeline.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12ComputePipeline : public IRHIComputePipeline
{
public:
    D3D12ComputePipeline(D3D12Device* device, RHIComputePipelineDesc desc);
    ~D3D12ComputePipeline();
};
