//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:54
//

#include "MetalDevice.h"
#include "MetalBLAS.h"
#include "MetalBuffer.h"
#include "MetalBufferView.h"
#include "MetalCommandQueue.h"
#include "MetalComputePipeline.h"
#include "MetalF2FSync.h"
#include "MetalGraphicsPipeline.h"
#include "MetalMeshPipeline.h"
#include "MetalSampler.h"
#include "MetalSurface.h"
#include "MetalTexture.h"
#include "MetalTextureView.h"
#include "MetalTLAS.h"
#include "MetalImGuiContext.h"

#include <Core/String.h>

MetalDevice::MetalDevice(bool validationLayers)
{
    mDevice = MTL::CreateSystemDefaultDevice();
    mDevice->retain();

    NS::String* deviceString = mDevice->name();
    SERAPH_INFO("Using Metal GPU: %s", deviceString->utf8String());
    deviceString->release();

    SERAPH_INFO("Created Metal device!");
}

MetalDevice::~MetalDevice()
{
    mDevice->release();
}

IRHISurface* MetalDevice::CreateSurface(Window* window, IRHICommandQueue* graphicsQueue)
{
    return (new MetalSurface(this, window, static_cast<MetalCommandQueue*>(graphicsQueue)));
}

IRHITexture* MetalDevice::CreateTexture(RHITextureDesc desc)
{
    return (new MetalTexture(this, desc));
}

IRHITextureView* MetalDevice::CreateTextureView(RHITextureViewDesc desc)
{
    return (new MetalTextureView(this, desc));
}

IRHICommandQueue* MetalDevice::CreateCommandQueue(RHICommandQueueType type)
{
    return (new MetalCommandQueue(this, type));
}

IRHIF2FSync* MetalDevice::CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue)
{
    return (new MetalF2FSync(this, static_cast<MetalSurface*>(surface), static_cast<MetalCommandQueue*>(queue)));
}

IRHIGraphicsPipeline* MetalDevice::CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc)
{
    return (new MetalGraphicsPipeline(this, desc));
}

IRHIBuffer* MetalDevice::CreateBuffer(RHIBufferDesc desc)
{
    return (new MetalBuffer(this, desc));
}

IRHISampler* MetalDevice::CreateSampler(RHISamplerDesc desc)
{
    return (new MetalSampler(this, desc));
}

IRHIComputePipeline* MetalDevice::CreateComputePipeline(RHIComputePipelineDesc desc)
{
    return (new MetalComputePipeline(this, desc));
}

IRHIMeshPipeline* MetalDevice::CreateMeshPipeline(RHIMeshPipelineDesc desc)
{
    return (new MetalMeshPipeline(this, desc));
}

IRHIBLAS* MetalDevice::CreateBLAS(RHIBLASDesc desc)
{
    return (new MetalBLAS(this, desc));
}

IRHITLAS* MetalDevice::CreateTLAS()
{
    return (new MetalTLAS(this));
}

IRHIBufferView* MetalDevice::CreateBufferView(RHIBufferViewDesc desc)
{
    return (new MetalBufferView(this, desc));
}

IRHIImGuiContext* MetalDevice::CreateImGuiContext(IRHICommandQueue* mainQueue, Window* window)
{
    return (new MetalImGuiContext(this, static_cast<MetalCommandQueue*>(mainQueue), window));
}
