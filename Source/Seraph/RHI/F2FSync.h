//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:40:09
//

/*
    The RHI has 2 kinds of synchronization : Queue sync, and F2F (frame to frame) sync. This is to distinguish how Vulkan and D3D12 approach synchronization differently.
    This system is temporary whilst I figure out a way to do Multi-Sync. Hopefully Timeline Semaphores are integrated into WSI as it would make things way easier for me.

    But Amelie! In what context should we use this!
    Since I want my RHI to also be usable for just generating a single image and then outputting it to disk, this is why I seperate it. F2F is your regular swapchain sync!

    Call BeginSynchronize at the beginning of the frame, EndSynchronize to submit your frame command buffer, and PresentSurface to present! Just as easy as that.
*/

#pragma once

#include <Core/Context.h>

#include "CommandList.h"

class IRHIDevice;

class IRHIF2FSync
{
public:
    ~IRHIF2FSync() = default;

    virtual uint BeginSynchronize() = 0;
    virtual void EndSynchronize(IRHICommandList* submitBuffer) = 0;
    virtual void PresentSurface() = 0;
};
