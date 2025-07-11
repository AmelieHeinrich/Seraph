//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:13:24
//

#include "MetalCommandQueue.h"
#include "MetalCommandList.h"
#include "MetalDevice.h"

MetalCommandQueue::MetalCommandQueue(MetalDevice* device, RHICommandQueueType type)
    : mParentDevice(device)
{
    SERAPH_WHATEVER("Created Metal command queue");
}

MetalCommandQueue::~MetalCommandQueue()
{

}

void MetalCommandQueue::SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer)
{

}

IRHICommandList* MetalCommandQueue::CreateCommandBuffer(bool singleTime)
{
    return (new MetalCommandList(mParentDevice, this, singleTime));
}
