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
    mCommandQueue = device->GetDevice()->newCommandQueue(16);
    if (!mCommandQueue) {
        SERAPH_ERROR("Failed to create command queue!");
    }
    mCommandQueue->addResidencySet(device->GetResidencySet());
    
    SERAPH_WHATEVER("Created Metal command queue");
}

MetalCommandQueue::~MetalCommandQueue()
{
}

void MetalCommandQueue::SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer)
{
    MetalCommandList* cmdList = static_cast<MetalCommandList*>(cmdBuffer);

    cmdList->GetBuffer()->commit();
}

IRHICommandList* MetalCommandQueue::CreateCommandBuffer(bool singleTime)
{
    return (new MetalCommandList(mParentDevice, this, singleTime));
}
