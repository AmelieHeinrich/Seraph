//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:54
//

#include "DummyDevice.h"
#include "DummyBLAS.h"
#include "DummyBuffer.h"
#include "DummyBufferView.h"
#include "DummyCommandQueue.h"
#include "DummyComputePipeline.h"
#include "DummyF2FSync.h"
#include "DummyGraphicsPipeline.h"
#include "DummyMeshPipeline.h"
#include "DummySampler.h"
#include "DummySurface.h"
#include "DummyTexture.h"
#include "DummyTextureView.h"
#include "DummyTLAS.h"
#include "DummyImGuiContext.h"

#include <Core/String.h>

DummyDevice::DummyDevice(bool validationLayers)
{
    SERAPH_INFO("Created Dummy device!");
}

DummyDevice::~DummyDevice()
{

}

IRHISurface* DummyDevice::CreateSurface(Window* window, IRHICommandQueue* graphicsQueue)
{
    return (new DummySurface(this, window, static_cast<DummyCommandQueue*>(graphicsQueue)));
}

IRHITexture* DummyDevice::CreateTexture(RHITextureDesc desc)
{
    return (new DummyTexture(this, desc));
}

IRHITextureView* DummyDevice::CreateTextureView(RHITextureViewDesc desc)
{
    return (new DummyTextureView(this, desc));
}

IRHICommandQueue* DummyDevice::CreateCommandQueue(RHICommandQueueType type)
{
    return (new DummyCommandQueue(this, type));
}

IRHIF2FSync* DummyDevice::CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue)
{
    return (new DummyF2FSync(this, static_cast<DummySurface*>(surface), static_cast<DummyCommandQueue*>(queue)));
}

IRHIGraphicsPipeline* DummyDevice::CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc)
{
    return (new DummyGraphicsPipeline(this, desc));
}

IRHIBuffer* DummyDevice::CreateBuffer(RHIBufferDesc desc)
{
    return (new DummyBuffer(this, desc));
}

IRHISampler* DummyDevice::CreateSampler(RHISamplerDesc desc)
{
    return (new DummySampler(this, desc));
}

IRHIComputePipeline* DummyDevice::CreateComputePipeline(RHIComputePipelineDesc desc)
{
    return (new DummyComputePipeline(this, desc));
}

IRHIMeshPipeline* DummyDevice::CreateMeshPipeline(RHIMeshPipelineDesc desc)
{
    return (new DummyMeshPipeline(this, desc));
}

IRHIBLAS* DummyDevice::CreateBLAS(RHIBLASDesc desc)
{
    return (new DummyBLAS(this, desc));
}

IRHITLAS* DummyDevice::CreateTLAS()
{
    return (new DummyTLAS(this));
}

IRHIBufferView* DummyDevice::CreateBufferView(RHIBufferViewDesc desc)
{
    return (new DummyBufferView(this, desc));
}

IRHIImGuiContext* DummyDevice::CreateImGuiContext(IRHICommandQueue* mainQueue, Window* window)
{
    return (new DummyImGuiContext(this, static_cast<DummyCommandQueue*>(mainQueue), window));
}
