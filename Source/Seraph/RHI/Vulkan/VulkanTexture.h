//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 14:31:27
//

#pragma once

#include <RHI/Texture.hpp>

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

class VulkanDevice;

class VulkanTexture : public IRHITexture
{
public:
    VulkanTexture(IRHIDevice* device, RHITextureDesc desc);
    ~VulkanTexture();

    void SetName(const StringView& name) override;

    VmaAllocation Allocation() const { return mAllocation; }
    VmaAllocationInfo AllocationInfo() const { return mAllocationInfo; }
    VkImage Image() const { return mImage; }

public:
    static VkFormat RHIToVkFormat(RHITextureFormat format);

private:
    VulkanDevice* mParentDevice;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    VkImage mImage;
};
