//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 14:35:21
//

#include "VulkanTexture.h"
#include "VulkanDevice.h"

VulkanTexture::VulkanTexture(IRHIDevice* device, RHITextureDesc desc)
    : mParentDevice(static_cast<VulkanDevice*>(device))
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = desc.Width;
    imageInfo.extent.height = desc.Height;
    imageInfo.mipLevels = desc.MipLevels;
    imageInfo.arrayLayers = desc.Depth;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.format = RHIToVkFormat(desc.Format);
    imageInfo.flags = 0;
    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (desc.Usage & RHITextureUsage::kRenderTarget) imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (desc.Usage & RHITextureUsage::kDepthTarget) imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (desc.Usage & RHITextureUsage::kShaderResource) imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (desc.Usage & RHITextureUsage::kStorage) imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    VkResult result = vmaCreateImage(mParentDevice->Allocator(), &imageInfo, &allocInfo, &mImage, &mAllocation, &mAllocationInfo);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create image!");
}

VulkanTexture::~VulkanTexture()
{
    if (mImage && mDesc.Reserved) vmaDestroyImage(mParentDevice->Allocator(), mImage, mAllocation);
}

void VulkanTexture::SetName(const StringView& name)
{
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectHandle = (uint64)mImage;
    nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    nameInfo.pObjectName = name.data();

    vkSetDebugUtilsObjectNameEXT(mParentDevice->Device(), &nameInfo);
}

VkFormat VulkanTexture::RHIToVkFormat(RHITextureFormat format)
{
    switch (format)
    {
        case RHITextureFormat::kR8G8B8A8_sRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case RHITextureFormat::kR8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_FORMAT_UNDEFINED;
}
