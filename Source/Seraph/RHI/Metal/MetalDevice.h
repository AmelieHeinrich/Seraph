//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:37
//

#pragma once

#include <RHI/Device.h>

#include <MetalCPP/Foundation/Foundation.hpp>
#include <MetalCPP/Metal/Metal.hpp>

class MetalDevice : public IRHIDevice
{
public:
    MetalDevice(bool validationLayers);
    ~MetalDevice();

    IRHISurface* CreateSurface(Window* window, IRHICommandQueue* graphicsQueue) override;
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
    IRHIImGuiContext* CreateImGuiContext(IRHICommandQueue* mainQueue, Window* window) override;

    RHITextureFormat GetSurfaceFormat() override { return RHITextureFormat::kB8G8R8A8_UNORM; }
    uint64 GetOptimalRowPitchAlignment() override { return 256; }
    uint64 GetBufferImageGranularity() override { return 1; }

    MTL::Device* GetDevice() { return mDevice; }
    MTL::ResidencySet* GetResidencySet() { return mResidencySet; }
private:
    MTL::Device* mDevice;
    MTL::ResidencySet* mResidencySet;
};
