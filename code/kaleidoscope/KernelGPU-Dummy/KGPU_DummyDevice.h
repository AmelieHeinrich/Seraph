//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:01:47
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

#include "KGPU_DummySurface.h"
#include "KGPU_DummyTexture.h"
#include "KGPU_DummyTextureView.h"
#include "KGPU_DummyCommandQueue.h"
#include "KGPU_DummySync.h"
#include "KGPU_DummyGraphicsPipeline.h"
#include "KGPU_DummyBuffer.h"
#include "KGPU_DummySampler.h"
#include "KGPU_DummyComputePipeline.h"
#include "KGPU_DummyBLAS.h"
#include "KGPU_DummyTLAS.h"
#include "KGPU_DummyBufferView.h"

namespace KGPU
{
    class DummyDevice : public IDevice
    {
    public:
        DummyDevice(bool validationLayers);
        ~DummyDevice();

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

        Backend GetBackend() { return Backend::kDummy; }

        TextureFormat GetSurfaceFormat() override { return TextureFormat::kR8G8B8A8_UNORM; }
        uint64 GetOptimalRowPitchAlignment() override { return 256; }
        uint64 GetBufferImageGranularity() override { return 1; }
        KGPU::ShaderBytecodeType GetTargetBytecode() override { return KGPU::ShaderBytecodeType::kSPIRV; }
    };
}
