//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 15:43:17
//

#include "VulkanTextureView.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

VulkanTextureView::VulkanTextureView(IRHIDevice* device, RHITextureViewDesc viewDesc)
    : mParentDevice(static_cast<VulkanDevice*>(device))
{
    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(viewDesc.Texture);
    RHITextureDesc desc = vkTexture->GetDesc();

    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = vkTexture->Image();
    createInfo.format = VulkanTexture::RHIToVkFormat(viewDesc.ViewFormat);
    createInfo.viewType = RHIToVkImageViewType(viewDesc.Dimension);
    createInfo.subresourceRange.aspectMask = viewDesc.Type == RHITextureViewType::kDepthTarget ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    if (viewDesc.ViewMip == VIEW_ALL_MIPS) {
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    } else {
        createInfo.subresourceRange.baseMipLevel = viewDesc.ViewMip;
        createInfo.subresourceRange.levelCount = 1;
    }
    createInfo.subresourceRange.baseArrayLayer = viewDesc.ArrayLayer;
    createInfo.subresourceRange.layerCount = desc.Depth;

    VkResult result = vkCreateImageView(mParentDevice->Device(), &createInfo, nullptr, &mImageView);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan image view!");
}

VulkanTextureView::~VulkanTextureView()
{
    if (mImageView) vkDestroyImageView(mParentDevice->Device(), mImageView, nullptr);
}

VkImageViewType VulkanTextureView::RHIToVkImageViewType(RHITextureViewDimension dimension)
{
    switch (dimension)
    {
        case RHITextureViewDimension::kTexture2D: return VK_IMAGE_VIEW_TYPE_2D;
        case RHITextureViewDimension::kTextureCube: return VK_IMAGE_VIEW_TYPE_CUBE;
    }
    return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
}
