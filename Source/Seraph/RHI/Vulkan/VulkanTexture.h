//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 14:31:27
//

#pragma once

#include <RHI/Texture.h>

#include <Vk/volk.h>
#include <vma/vk_mem_alloc.h>

class VulkanDevice;

class VulkanTexture : public IRHITexture
{
public:
    VulkanTexture() = default;
    VulkanTexture(IRHIDevice* device, RHITextureDesc desc);
    ~VulkanTexture();

    void SetName(const String& name) override;

    VmaAllocation Allocation() const { return mAllocation; }
    VmaAllocationInfo AllocationInfo() const { return mAllocationInfo; }
    VkImage Image() const { return mImage; }

public:
    static VkFormat RHIToVkFormat(RHITextureFormat format);

private:
    friend class VulkanSurface;

    VulkanDevice* mParentDevice;

    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;

    VkImage mImage;
};
