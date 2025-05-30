//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 21:59:29
//

#include "VulkanBuffer.h"
#include "VulkanDevice.h"

VulkanBuffer::VulkanBuffer(VulkanDevice* device, RHIBufferDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = desc.Size;
    bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    bufferInfo.usage
}

VulkanBuffer::~VulkanBuffer()
{

}

void VulkanBuffer::SetName(const StringView& name)
{

}
