//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:58:44
//

#include "KGPU_VulkanTexture.h"
#include "KGPU_VulkanDevice.h"

namespace KGPU
{
    VulkanTexture::VulkanTexture(TextureDesc desc)
    {
        mDesc = desc;
        for (int i = 0; i < desc.MipLevels; i++) {
            mLayouts.push_back(KGPU::ResourceLayout::kUndefined);
        }
    }
    
    VulkanTexture::VulkanTexture(VulkanDevice* device, TextureDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;
        for (int i = 0; i < desc.MipLevels; i++) {
            mLayouts.push_back(KGPU::ResourceLayout::kUndefined);
        }

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = desc.Width;
        imageInfo.extent.height = desc.Height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = desc.MipLevels;
        imageInfo.arrayLayers = desc.Depth;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.format = RHIToVkFormat(desc.Format);
        imageInfo.flags = 0;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (Any(desc.Usage & TextureUsage::kRenderTarget)) imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (Any(desc.Usage & TextureUsage::kDepthTarget)) imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (Any(desc.Usage & TextureUsage::kShaderResource)) imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (Any(desc.Usage & TextureUsage::kStorage)) imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (ITexture::ToSRGB(desc.Format) != desc.Format) {
            imageInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;;
        }
        if (desc.Depth == 6) {
            imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        VkResult result = vmaCreateImage(mParentDevice->Allocator(), &imageInfo, &allocInfo, &mImage, &mAllocation, &mAllocationInfo);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create image!");
        
        KD_WHATEVER("Created Vulkan texture");
    }
    
    VulkanTexture::~VulkanTexture()
    {
        if (mImage && !mDesc.Reserved) vmaDestroyImage(mParentDevice->Allocator(), mImage, mAllocation);
    }
    
    void VulkanTexture::SetName(const KC::String& name)
    {
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectHandle = (uint64)mImage;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
        nameInfo.pObjectName = name.data();

        vkSetDebugUtilsObjectNameEXT(mParentDevice->Device(), &nameInfo);
    }

    VkFormat VulkanTexture::RHIToVkFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::kR8G8B8A8_sRGB: return VK_FORMAT_R8G8B8A8_SRGB;
            case TextureFormat::kR8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::kB8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
            case TextureFormat::kD32_FLOAT: return VK_FORMAT_D32_SFLOAT;
            case TextureFormat::kR16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
            case TextureFormat::kR16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
            case TextureFormat::kR32_FLOAT: return VK_FORMAT_R32_SFLOAT;
            case TextureFormat::kBC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
            case TextureFormat::kBC7_sRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
            case TextureFormat::kR16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
            case TextureFormat::kR16_FLOAT: return VK_FORMAT_R16_SFLOAT;
            case TextureFormat::kR8_UINT: return VK_FORMAT_R8_UINT;
            case TextureFormat::kR16_UINT: return VK_FORMAT_R16_UINT;
        }
        return VK_FORMAT_UNDEFINED;
    }
}
