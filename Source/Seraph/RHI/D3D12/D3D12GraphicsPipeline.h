//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:58:11
//

#pragma once

#include <RHI/GraphicsPipeline.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12GraphicsPipeline : public IRHIGraphicsPipeline
{
public:
    D3D12GraphicsPipeline(D3D12Device* device, RHIGraphicsPipelineDesc desc);
    ~D3D12GraphicsPipeline();
};
