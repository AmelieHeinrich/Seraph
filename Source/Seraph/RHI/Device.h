//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:26:45
//

#pragma once

#include <Core/Context.h>

#include "Backend.h"
#include "Surface.h"
#include "Texture.h"
#include "TextureView.h"
#include "CommandQueue.h"
#include "F2FSync.h"
#include "GraphicsPipeline.h"
#include "Buffer.h"

class IRHIDevice
{
public:
    ~IRHIDevice() = default;

    static IRHIDevice* CreateDevice(RHIBackend backend, bool validationLayers);

    virtual IRHISurface* CreateSurface(Window* window) = 0;
    virtual IRHITexture* CreateTexture(RHITextureDesc desc) = 0;
    virtual IRHITextureView* CreateTextureView(RHITextureViewDesc desc) = 0;
    virtual IRHICommandQueue* CreateCommandQueue(RHICommandQueueType type) = 0;
    virtual IRHIF2FSync* CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue) = 0;
    virtual IRHIGraphicsPipeline* CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc) = 0;
    virtual IRHIBuffer* CreateBuffer(RHIBufferDesc desc) = 0;
protected:
    IRHIDevice() = default;
};
