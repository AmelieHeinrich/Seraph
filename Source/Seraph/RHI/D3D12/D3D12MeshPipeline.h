//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:06:50
//

#pragma once

#include <RHI/MeshPipeline.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12MeshPipeline : public IRHIMeshPipeline
{
public:
    D3D12MeshPipeline(D3D12Device* device, RHIMeshPipelineDesc desc);
    ~D3D12MeshPipeline();
};
