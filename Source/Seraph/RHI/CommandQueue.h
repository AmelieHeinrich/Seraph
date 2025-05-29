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

class IRHICommandQueue
{
public:
    ~IRHICommandQueue() = default;

    RHICommandQueueType GetType() const { return mType; }

protected:
    RHICommandQueueType mType;
};
