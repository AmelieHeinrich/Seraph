//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:11:40
//

#pragma once

#include <KernelGPU/KGPU_GraphicsPipeline.h>

#include <Agility/d3d12.h>

namespace KGPU
{
    class D3D12Device;

    class D3D12GraphicsPipeline : public IGraphicsPipeline
    {
    public:
        D3D12GraphicsPipeline(D3D12Device* device, GraphicsPipelineDesc desc);
        ~D3D12GraphicsPipeline();

        ID3D12PipelineState* GetPipelineState() { return mPipelineState; }
        ID3D12RootSignature* GetRootSignature() { return mRootSignature; }

    private:
        D3D12_COMPARISON_FUNC ToD3DCompareOp(DepthOperation op);
        D3D12_CULL_MODE ToD3DCullMode(CullMode mode);
        D3D12_FILL_MODE ToD3DFillMode(FillMode mode);

    private:
        ID3D12PipelineState* mPipelineState;
        ID3D12RootSignature* mRootSignature;
    };
}
