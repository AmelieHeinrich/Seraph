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

    IRHISurface* CreateSurface(Window* window) override;
    IRHITexture* CreateTexture(RHITextureDesc desc) override;
    IRHITextureView* CreateTextureView(RHITextureViewDesc desc) override;
    IRHICommandQueue* CreateCommandQueue(RHICommandQueueType type) override;
    IRHIF2FSync* CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue) override;
    IRHIGraphicsPipeline* CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc) override;
    IRHIBuffer* CreateBuffer(RHIBufferDesc desc) override;
    IRHISampler* CreateSampler(RHISamplerDesc desc) override;
    IRHIComputePipeline* CreateComputePipeline(RHIComputePipelineDesc desc) override;
    IRHIMeshPipeline* CreateMeshPipeline(RHIMeshPipelineDesc desc) override;
    IRHIBLAS* CreateBLAS(RHIBLASDesc desc) override;
    IRHITLAS* CreateTLAS() override;
    IRHIBufferView* CreateBufferView(RHIBufferViewDesc desc) override;
};
