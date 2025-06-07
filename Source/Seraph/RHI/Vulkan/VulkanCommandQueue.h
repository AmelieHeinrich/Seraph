//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:51:03
//

#pragma once

#include <RHI/CommandQueue.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanCommandQueue : public IRHICommandQueue
{
public:
    VulkanCommandQueue(IRHIDevice* device, RHICommandQueueType type);
    ~VulkanCommandQueue();

    IRHICommandList* CreateCommandBuffer(bool singleTime) override;

    void SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer) override;
public:
    VkQueue GetQueue() { return mQueue; }

private:
    VulkanDevice* mParentDevice;

    VkQueue mQueue;
    VkCommandPool mCommandPool;
};
