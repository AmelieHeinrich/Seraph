//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:22:00
//

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, bool singleTime)
    : mParentDevice(device), mParentPool(pool)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    VkResult result = vkAllocateCommandBuffers(device->Device(), &allocateInfo, &mCmdBuffer);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to allocate command buffer!");
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    if (mCmdBuffer) vkFreeCommandBuffers(mParentDevice->Device(), mParentPool, 1, &mCmdBuffer);
}
