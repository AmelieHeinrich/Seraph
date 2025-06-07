//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:49:13
//

#pragma once

#include <Core/Context.h>

enum class RHICommandQueueType
{
    kGraphics,
    kCompute,
    kCopy
};

class IRHIDevice;
class IRHICommandList;

class IRHICommandQueue
{
public:
    ~IRHICommandQueue() = default;

    RHICommandQueueType GetType() const { return mType; }

    virtual IRHICommandList* CreateCommandBuffer(bool singleTime) = 0;

    virtual void SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer) = 0;
protected:
    RHICommandQueueType mType;
};
