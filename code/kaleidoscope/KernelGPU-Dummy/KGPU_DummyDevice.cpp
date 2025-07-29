//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:04:09
//

#include "KGPU_DummyDevice.h"

#include <KernelCore/KC_Assert.h>
#include <unordered_map>

namespace KGPU
{
    DummyDevice::DummyDevice(bool validationLayers)
    {
        KD_INFO("Created Dummy device!");
    }

    DummyDevice::~DummyDevice()
    {
    }

    ISurface* DummyDevice::CreateSurface(KOS::IWindow* window, ICommandQueue* graphicsQueue)
    {
        return KC_NEW(DummySurface, this, window, static_cast<DummyCommandQueue*>(graphicsQueue));
    }

    ITexture* DummyDevice::CreateTexture(TextureDesc desc)
    {
        return KC_NEW(DummyTexture, this, desc);
    }

    ITextureView* DummyDevice::CreateTextureView(TextureViewDesc desc)
    {
        return KC_NEW(DummyTextureView, this, desc);
    }

    ICommandQueue* DummyDevice::CreateCommandQueue(CommandQueueType type)
    {
        return KC_NEW(DummyCommandQueue, this, type);
    }

    ISync* DummyDevice::CreateSync(ISurface* surface, ICommandQueue* queue)
    {
    return KC_NEW(DummySync, this, static_cast<DummySurface*>(surface), static_cast<DummyCommandQueue*>(queue));
    }

    IGraphicsPipeline* DummyDevice::CreateGraphicsPipeline(GraphicsPipelineDesc desc)
    {
        return KC_NEW(DummyGraphicsPipeline, this, desc);
    }

    IBuffer* DummyDevice::CreateBuffer(BufferDesc desc)
    {
        return KC_NEW(DummyBuffer, this, desc);
    }

    ISampler* DummyDevice::CreateSampler(SamplerDesc desc)
    {
        return KC_NEW(DummySampler, this, desc);
    }

    IComputePipeline* DummyDevice::CreateComputePipeline(ComputePipelineDesc desc)
    {
        return KC_NEW(DummyComputePipeline, this, desc);
    }

    IBLAS* DummyDevice::CreateBLAS(BLASDesc desc)
    {
        return KC_NEW(DummyBLAS, this, desc);
    }

    ITLAS* DummyDevice::CreateTLAS()
    {
        return KC_NEW(DummyTLAS, this);
    }

    IBufferView* DummyDevice::CreateBufferView(BufferViewDesc desc)
    {
        return KC_NEW(DummyBufferView, this, desc);
    }
}
