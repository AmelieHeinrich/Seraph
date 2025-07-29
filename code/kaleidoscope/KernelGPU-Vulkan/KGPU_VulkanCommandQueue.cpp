//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:36:04
//

#include "KGPU_VulkanCommandQueue.h"
#include "KGPU_VulkanDevice.h"
#include "KGPU_VulkanCommandList.h"

namespace KGPU
{
    VulkanCommandQueue::VulkanCommandQueue(VulkanDevice* device, CommandQueueType type)
        : mParentDevice(device)
    {
        uint32 queueFamilyIndex = 0;
        switch (type)
        {
            case CommandQueueType::kGraphics: queueFamilyIndex = mParentDevice->GraphicsQueueFamilyIndex(); break;
            case CommandQueueType::kCompute: queueFamilyIndex = mParentDevice->ComputeQueueFamilyIndex(); break;
            case CommandQueueType::kCopy: queueFamilyIndex = mParentDevice->TransferQueueFamilyIndex(); break;
        }

        vkGetDeviceQueue(mParentDevice->Device(), queueFamilyIndex, 0, &mQueue);

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        VkResult result = vkCreateCommandPool(mParentDevice->Device(), &poolInfo, nullptr, &mCommandPool);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create command pool!");
    }

    VulkanCommandQueue::~VulkanCommandQueue()
    {
        if (mCommandPool) vkDestroyCommandPool(mParentDevice->Device(), mCommandPool, nullptr);
    }

    ICommandList* VulkanCommandQueue::CreateCommandList(bool singleTime)
    {
        return (KC_NEW(VulkanCommandList, mParentDevice, mCommandPool, singleTime));
    }

    void VulkanCommandQueue::CommitCommandList(ICommandList* cmdBuffer)
    {
        VulkanCommandList* vkCmdBuffer = static_cast<VulkanCommandList*>(cmdBuffer);
        VkCommandBuffer vkCmd = vkCmdBuffer->GetCommandBuffer();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vkCmd;

        VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to submit and flush command buffer!");

        Wait();
    }

    void VulkanCommandQueue::Wait()
    {
        vkQueueWaitIdle(mQueue);
    }
}
