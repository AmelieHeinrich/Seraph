//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:06:29
//

#pragma once

#include "CommandQueue.h"

class IRHICommandBuffer
{
public:
    ~IRHICommandBuffer() = default;

    IRHICommandQueue* GetParentQueue() { return mParentQueue; }

protected:
    IRHICommandQueue* mParentQueue;
};
