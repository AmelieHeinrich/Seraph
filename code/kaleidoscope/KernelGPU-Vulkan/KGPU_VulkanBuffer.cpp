//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:31:59
//

#include "KGPU_VulkanBuffer.h"
#include "KGPU_VulkanDevice.h"

namespace KGPU
{
    VulkanBuffer::VulkanBuffer(VulkanDevice* device, BufferDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.Size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (Any(desc.Usage & BufferUsage::kVertex)) bufferInfo.usage |= (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
        if (Any(desc.Usage & BufferUsage::kIndex)) bufferInfo.usage |= (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
        if (Any(desc.Usage & BufferUsage::kConstant)) bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        if (Any(desc.Usage & BufferUsage::kShaderRead) || Any(desc.Usage & BufferUsage::kShaderWrite)) bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (Any(desc.Usage & BufferUsage::kAccelerationStructure)) bufferInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
        if (Any(desc.Usage & BufferUsage::kShaderBindingTable)) bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;

        if (mParentDevice->SupportsRaytracing()) {
            bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
        }

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        if (Any(desc.Usage & BufferUsage::kStaging)) allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
        if (Any(desc.Usage & BufferUsage::kReadback)) allocInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        if (Any(desc.Usage & BufferUsage::kConstant)) allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        VkResult result = vmaCreateBuffer(mParentDevice->Allocator(), &bufferInfo, &allocInfo, &mBuffer, &mAllocation, &mAllocationInfo);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to allocate Vulkan buffer!");

        KD_WHATEVER("Created Vulkan buffer");
    }

    VulkanBuffer::~VulkanBuffer()
    {
        vmaDestroyBuffer(mParentDevice->Allocator(), mBuffer, mAllocation); 
    }

    void VulkanBuffer::SetName(const KC::String& name)
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

    void* VulkanBuffer::Map()
    {
        void* ptr;
        vmaMapMemory(mParentDevice->Allocator(), mAllocation, &ptr);
        return ptr;
    }

    void VulkanBuffer::Unmap(uint start, uint end)
    {
        vmaUnmapMemory(mParentDevice->Allocator(), mAllocation);
    }

    uint64 VulkanBuffer::GetAddress()
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = mBuffer;
        
        return vkGetBufferDeviceAddress(mParentDevice->Device(), &info);
    }
}
