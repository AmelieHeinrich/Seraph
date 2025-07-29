//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:36:04
//

#include "KGPU_DummyCommandQueue.h"
#include "KGPU_DummyDevice.h"
#include "KGPU_DummyCommandList.h"

namespace KGPU
{
    DummyCommandQueue::DummyCommandQueue(DummyDevice* device, CommandQueueType type)
        : mParentDevice(device)
    {
    }

    DummyCommandQueue::~DummyCommandQueue()
    {
    }

    ICommandList* DummyCommandQueue::CreateCommandList(bool singleTime)
    {
        return (KC_NEW(DummyCommandList, mParentDevice, this, singleTime));
    }

    void DummyCommandQueue::CommitCommandList(ICommandList* cmdBuffer)
    {
    }

    void DummyCommandQueue::Wait()
    {
    }
}
