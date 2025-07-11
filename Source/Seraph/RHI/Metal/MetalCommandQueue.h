//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:51:03
//

#pragma once

#include <RHI/CommandQueue.h>

#include <MetalCPP/Metal/Metal.hpp>

class MetalDevice;

class MetalCommandQueue : public IRHICommandQueue
{
public:
    MetalCommandQueue(MetalDevice* device, RHICommandQueueType type);
    ~MetalCommandQueue();

    IRHICommandList* CreateCommandBuffer(bool singleTime) override;

    void SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer) override;

private:
    MetalDevice* mParentDevice;

    MTL::CommandQueue* mCommandQueue;
};
