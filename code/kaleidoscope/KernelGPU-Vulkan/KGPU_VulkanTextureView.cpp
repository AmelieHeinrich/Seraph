//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:02:44
//

#include "KGPU_VulkanTextureView.h"
#include "KGPU_VulkanDevice.h"

namespace KGPU
{
    VulkanTextureView::VulkanTextureView(VulkanDevice* device, TextureViewDesc viewDesc)
        : mParentDevice(device)
    {
        mDesc = viewDesc;

        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(viewDesc.Texture);
        TextureDesc desc = vkTexture->GetDesc();

        VkFormat format = VulkanTexture::RHIToVkFormat(viewDesc.ViewFormat);
        if (Any(desc.Usage & TextureUsage::kDepthTarget)) format = VulkanTexture::RHIToVkFormat(desc.Format);

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vkTexture->Image();
        createInfo.format = format;
        createInfo.viewType = RHIToVkImageViewType(viewDesc.Dimension, viewDesc.Type);
        createInfo.subresourceRange.aspectMask = Any(desc.Usage & TextureUsage::kDepthTarget) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        if (viewDesc.ViewMip == VIEW_ALL_MIPS) {
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
        } else {
            createInfo.subresourceRange.baseMipLevel = viewDesc.ViewMip;
            createInfo.subresourceRange.levelCount = 1;
        }
        createInfo.subresourceRange.baseArrayLayer = viewDesc.ArrayLayer == VIEW_ALL_MIPS ? 0 : viewDesc.ArrayLayer;
        createInfo.subresourceRange.layerCount = viewDesc.ArrayLayer == VIEW_ALL_MIPS ? desc.Depth : 1;

        VkResult result = vkCreateImageView(mParentDevice->Device(), &createInfo, nullptr, &mImageView);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan image view!");

        // Get bindless
        switch (viewDesc.Type) {
            case TextureViewType::kShaderRead: {
                mBindless.Index = mParentDevice->GetBindlessManager()->WriteTextureSRV(this);
                break;
            }
            case TextureViewType::kShaderWrite: {
                mBindless.Index = mParentDevice->GetBindlessManager()->WriteTextureUAV(this);
                break;
            }
        }

        KD_WHATEVER("Created Vulkan texture view");
    }

    VulkanTextureView::~VulkanTextureView()
    {
        if (mBindless.Index != BINDLESS_INVALID_HANDLE) mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mBindless.Index);
        if (mImageView) vkDestroyImageView(mParentDevice->Device(), mImageView, nullptr);
    }

    VkImageViewType VulkanTextureView::RHIToVkImageViewType(TextureViewDimension dimension, TextureViewType type)
    {
        switch (dimension)
        {
            case TextureViewDimension::kTexture2D: return VK_IMAGE_VIEW_TYPE_2D;
            case TextureViewDimension::kTextureCube: return (type == TextureViewType::kShaderWrite) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        }
        return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}
