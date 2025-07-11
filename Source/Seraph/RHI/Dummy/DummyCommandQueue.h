//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:51:03
//

#pragma once

#include <RHI/CommandQueue.h>

class DummyDevice;

class DummyCommandQueue : public IRHICommandQueue
{
public:
    DummyCommandQueue(DummyDevice* device, RHICommandQueueType type);
    ~DummyCommandQueue();

    IRHICommandList* CreateCommandBuffer(bool singleTime) override;

    void SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer) override;

private:
    DummyDevice* mParentDevice;
};
