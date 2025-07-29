//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:01:47
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

#include <Agility/d3d12.h>
#include <dxgi1_6.h>

#include "KGPU_D3D12Surface.h"
#include "KGPU_D3D12Texture.h"
#include "KGPU_D3D12TextureView.h"
#include "KGPU_D3D12CommandQueue.h"
#include "KGPU_D3D12Sync.h"
#include "KGPU_D3D12GraphicsPipeline.h"
#include "KGPU_D3D12Buffer.h"
#include "KGPU_D3D12Sampler.h"
#include "KGPU_D3D12ComputePipeline.h"
#include "KGPU_D3D12BLAS.h"
#include "KGPU_D3D12TLAS.h"
#include "KGPU_D3D12BufferView.h"
#include "KGPU_D3D12BindlessManager.h"

namespace KGPU
{
    class D3D12Device : public IDevice
    {
    public:
        D3D12Device(bool validationLayers);
        ~D3D12Device();

        ISurface* CreateSurface(KOS::IWindow* window, ICommandQueue* graphicsQueue) override;
        ITexture* CreateTexture(TextureDesc desc) override;
        ITextureView* CreateTextureView(TextureViewDesc desc) override;
        ICommandQueue* CreateCommandQueue(CommandQueueType type) override;
        ISync* CreateSync(ISurface* surface, ICommandQueue* queue) override;
        IGraphicsPipeline* CreateGraphicsPipeline(GraphicsPipelineDesc desc) override;
        IBuffer* CreateBuffer(BufferDesc desc) override;
        ISampler* CreateSampler(SamplerDesc desc) override;
        IComputePipeline* CreateComputePipeline(ComputePipelineDesc desc) override;
        IBLAS* CreateBLAS(BLASDesc desc) override;
        ITLAS* CreateTLAS() override;
        IBufferView* CreateBufferView(BufferViewDesc desc) override;

        Backend GetBackend() { return Backend::kD3D12; }
        bool SupportsRaytracing() { return mSupportsRT; }
        bool SupportsMeshShaders() { return mSupportsMS; }

        TextureFormat GetSurfaceFormat() override { return TextureFormat::kR8G8B8A8_UNORM; }
        uint64 GetOptimalRowPitchAlignment() override { return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT; }
        uint64 GetBufferImageGranularity() override { return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT; }
        KGPU::ShaderBytecodeType GetTargetBytecode() override { return KGPU::ShaderBytecodeType::kDXIL; }
    public:
        ID3D12Device14* GetDevice() { return mDevice; }
        IDXGIFactory6* GetFactory() { return mFactory; }

        D3D12BindlessManager* GetBindlessManager() { return mManager; }
    
    private:
        IDXGIFactory6* mFactory = nullptr;
        IDXGIAdapter1* mAdapter = nullptr;
        ID3D12Device14* mDevice = nullptr;
        ID3D12Debug1* mDebug = nullptr;
        ID3D12InfoQueue1* mInfoQueue = nullptr;

        D3D12BindlessManager* mManager;
        bool mSupportsRT = false;
        bool mSupportsMS = false;

        uint64 CalculateAdapterScore(IDXGIAdapter1* adapter);
    };
}
