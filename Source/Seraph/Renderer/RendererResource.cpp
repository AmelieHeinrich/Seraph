//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 12:55:48
//

#include "RendererResource.h"

RendererResource::~RendererResource()
{
    switch (Type)
    {
        case RendererResourceType::kBuffer: {
            delete Buffer;
            break;
        }
        case RendererResourceType::kRingBuffer: {
            for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
                delete RingBufferViews[i];
                delete RingBuffer[i];
            }
            break;
        }
        case RendererResourceType::kTexture: {
            delete Texture;
            break;
        }
        case RendererResourceType::kSampler: {
            delete Sampler;
            break;
        }
    }
}
