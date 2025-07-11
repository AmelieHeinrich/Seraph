//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:13:24
//

#include "DummyCommandQueue.h"
#include "DummyCommandList.h"
#include "DummyDevice.h"

DummyCommandQueue::DummyCommandQueue(DummyDevice* device, RHICommandQueueType type)
    : mParentDevice(device)
{
    SERAPH_WHATEVER("Created Dummy command queue");
}

DummyCommandQueue::~DummyCommandQueue()
{

}

void DummyCommandQueue::SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer)
{

}

IRHICommandList* DummyCommandQueue::CreateCommandBuffer(bool singleTime)
{
    return (new DummyCommandList(mParentDevice, this, singleTime));
}
