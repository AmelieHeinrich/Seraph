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

    IRHIBuffer* Buffer = nullptr;
    StaticArray<IRHIBuffer*, FRAMES_IN_FLIGHT> RingBuffer;
    StaticArray<IRHIBufferView*, FRAMES_IN_FLIGHT> RingBufferViews;
    IRHITexture* Texture = nullptr;
    IRHISampler* Sampler = nullptr;

    RHIResourceAccess LastAccess = RHIResourceAccess::kNone;
    RHIPipelineStage LastStage = RHIPipelineStage::kNone;

    ~RendererResource();
};
