//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:33:34
//

#pragma once

#include "KGPU_CommandQueue.h"

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanCommandQueue : public ICommandQueue
    {
    public:
        VulkanCommandQueue(VulkanDevice* device, CommandQueueType type);
        ~VulkanCommandQueue();

        ICommandList* CreateCommandList(bool singleTime) override;
        void CommitCommandList(ICommandList* cmdBuffer) override;
        void Wait() override;
        
    public:
        VkQueue GetQueue() { return mQueue; }
    
    private:
        VulkanDevice* mParentDevice;

        VkQueue mQueue;
        VkCommandPool mCommandPool;
    };
}
