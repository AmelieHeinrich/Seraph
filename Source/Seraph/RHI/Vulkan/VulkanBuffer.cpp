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
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; 
    if (Any(desc.Usage & RHIBufferUsage::kVertex)) bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (Any(desc.Usage & RHIBufferUsage::kIndex)) bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (Any(desc.Usage & RHIBufferUsage::kConstant)) bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (Any(desc.Usage & RHIBufferUsage::kShaderRead) || Any(desc.Usage & RHIBufferUsage::kShaderWrite)) bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (Any(desc.Usage & RHIBufferUsage::kAccelerationStructure)) bufferInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    if (Any(desc.Usage & RHIBufferUsage::kShaderBindingTable)) bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    if (Any(desc.Usage & RHIBufferUsage::kStaging)) allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    if (Any(desc.Usage & RHIBufferUsage::kReadback)) allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    if (Any(desc.Usage & RHIBufferUsage::kConstant)) allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VkResult result = vmaCreateBuffer(mParentDevice->Allocator(), &bufferInfo, &allocInfo, &mBuffer, &mAllocation, &mAllocationInfo);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to allocate Vulkan buffer!");
}

VulkanBuffer::~VulkanBuffer()
{
    vmaDestroyBuffer(mParentDevice->Allocator(), mBuffer, mAllocation);
}

void VulkanBuffer::SetName(const StringView& name)
{
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = NULL,
        .objectType = VK_OBJECT_TYPE_BUFFER,
        .objectHandle = (uint64)mBuffer,
        .pObjectName = name.data(),
    };

    vkSetDebugUtilsObjectNameEXT(mParentDevice->Device(), &nameInfo);
}
