//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:37
//

#pragma once

#include <RHI/Device.h>

#include <Agility/d3d12.h>
#include <dxgi1_6.h>
#include <D3D12MA/D3D12MemAlloc.h>

#include "D3D12BindlessManager.h"

class D3D12Device : public IRHIDevice
{
public:
    D3D12Device(bool validationLayers);
    ~D3D12Device();

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

public:
    ID3D12Device14* GetDevice() { return mDevice; }
    IDXGIFactory6* GetFactory() { return mFactory; }
    D3D12BindlessManager* GetBindlessManager() { return mBindlessManager; }

private:
    IDXGIFactory6* mFactory = nullptr;
    IDXGIAdapter1* mAdapter = nullptr;
    ID3D12Device14* mDevice = nullptr;
    ID3D12Debug1* mDebug = nullptr;

    D3D12BindlessManager* mBindlessManager;
    D3D12MA::Allocator* mAllocator;

    uint64 CalculateAdapterScore(IDXGIAdapter1* adapter);
};
