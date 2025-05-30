//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 21:56:11
//

#pragma once

#include <RHI/Buffer.h>

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

class VulkanDevice;

class VulkanBuffer : public IRHIBuffer
{
public:
    VulkanBuffer(VulkanDevice* device, RHIBufferDesc desc);
    ~VulkanBuffer();

    void SetName(const StringView& name) override;

private:
    VulkanDevice* mParentDevice;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    VkBuffer mBuffer;
};
