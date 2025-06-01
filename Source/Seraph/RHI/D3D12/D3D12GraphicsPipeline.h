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

    ID3D12PipelineState* GetPipelineState() { return mPipelineState; }
    ID3D12RootSignature* GetRootSignature() { return mRootSignature; }

private:
    D3D12_COMPARISON_FUNC ToD3DCompareOp(RHIDepthOperation op);
    D3D12_CULL_MODE ToD3DCullMode(RHICullMode mode);
    D3D12_FILL_MODE ToD3DFillMode(RHIFillMode mode);

private:
    ID3D12PipelineState* mPipelineState;
    ID3D12RootSignature* mRootSignature;
};
