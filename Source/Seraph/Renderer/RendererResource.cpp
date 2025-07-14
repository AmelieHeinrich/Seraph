//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 12:55:48
//

#include "RendererResource.h"

RendererResource::~RendererResource()
{
    if (Buffer) delete Buffer;
    if (RingBuffer[0]) {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            if (RingBufferViews[i]) delete RingBufferViews[i];
            if (RingBuffer[i]) delete RingBuffer[i];
        }
    }
    if (Texture) delete Texture;
    if (Sampler) delete Sampler;
}
