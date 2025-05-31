//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:37
//

#pragma once

#include <RHI/Device.h>

class D3D12Device : public IRHIDevice
{
public:
    D3D12Device(bool validationLayers);
    ~D3D12Device();

    IRHISurface* CreateSurface(Window* window) override { return nullptr; }
    IRHITexture* CreateTexture(RHITextureDesc desc) override { return nullptr; }
    IRHITextureView* CreateTextureView(RHITextureViewDesc desc) override { return nullptr; }
    IRHICommandQueue* CreateCommandQueue(RHICommandQueueType type) override;
    IRHIF2FSync* CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue) override { return nullptr; }
    IRHIGraphicsPipeline* CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc) override { return nullptr; }
    IRHIBuffer* CreateBuffer(RHIBufferDesc desc) override { return nullptr; }
    IRHISampler* CreateSampler(RHISamplerDesc desc) override { return nullptr; }
    IRHIComputePipeline* CreateComputePipeline(RHIComputePipelineDesc desc) override { return nullptr; }
    IRHIMeshPipeline* CreateMeshPipeline(RHIMeshPipelineDesc desc) override { return nullptr; }
};
