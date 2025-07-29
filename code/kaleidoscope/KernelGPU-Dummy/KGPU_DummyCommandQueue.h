//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:33:34
//

#pragma once

#include "KGPU_CommandQueue.h"

namespace KGPU
{
    class DummyDevice;

    class DummyCommandQueue : public ICommandQueue
    {
    public:
        DummyCommandQueue(DummyDevice* device, CommandQueueType type);
        ~DummyCommandQueue();

        ICommandList* CreateCommandList(bool singleTime) override;
        void CommitCommandList(ICommandList* cmdBuffer) override;
        void Wait() override;
        
    private:
        DummyDevice* mParentDevice;
    };
}
