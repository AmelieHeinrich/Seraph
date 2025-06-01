//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:53:05
//

#include "VulkanCommandQueue.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

VulkanCommandQueue::VulkanCommandQueue(IRHIDevice* device, RHICommandQueueType type)
    : mParentDevice(static_cast<VulkanDevice*>(device))
{
    uint32 queueFamilyIndex = 0;
    switch (type)
    {
        case RHICommandQueueType::kGraphics: queueFamilyIndex = mParentDevice->GraphicsQueueFamilyIndex(); break;
        case RHICommandQueueType::kCompute: queueFamilyIndex = mParentDevice->ComputeQueueFamilyIndex(); break;
        case RHICommandQueueType::kCopy: queueFamilyIndex = mParentDevice->TransferQueueFamilyIndex(); break;
    }

    vkGetDeviceQueue(mParentDevice->Device(), queueFamilyIndex, 0, &mQueue);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VkResult result = vkCreateCommandPool(mParentDevice->Device(), &poolInfo, nullptr, &mCommandPool);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create command pool!");

    SERAPH_WHATEVER("Created Vulkan command queue");
}

VulkanCommandQueue::~VulkanCommandQueue()
{
    if (mCommandPool) vkDestroyCommandPool(mParentDevice->Device(), mCommandPool, nullptr);
}

IRHICommandBuffer* VulkanCommandQueue::CreateCommandBuffer(bool singleTime)
{
    return new VulkanCommandBuffer(mParentDevice, mCommandPool, singleTime);
}

void VulkanCommandQueue::SubmitAndFlushCommandBuffer(IRHICommandBuffer* cmdBuffer)
{
    VulkanCommandBuffer* vkCmdBuffer = static_cast<VulkanCommandBuffer*>(cmdBuffer);
    VkCommandBuffer vkCmd = vkCmdBuffer->GetCommandBuffer();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCmd;

    VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, VK_NULL_HANDLE);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to submit and flush command buffer!");

    vkQueueWaitIdle(mQueue);
}
