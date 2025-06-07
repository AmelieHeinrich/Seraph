//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 12:52:22
//

#pragma once

#include <RHI/Device.h>

enum class RendererResourceType
{
    kBuffer,
    kRingBuffer,
    kTexture,
    kSampler
};

struct RendererResource
{
    RendererResourceType Type;

    IRHIBuffer* Buffer;
    IRHIBuffer* RingBuffer[FRAMES_IN_FLIGHT];
    IRHIBufferView* RingBufferViews[FRAMES_IN_FLIGHT];
    IRHITexture* Texture;
    IRHISampler* Sampler;

    RHIResourceAccess LastAccess;
    RHIPipelineStage LastStage;

    ~RendererResource();
};
