//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 15:02:00
//

#include "VulkanSampler.h"
#include "VulkanDevice.h"

VulkanSampler::VulkanSampler(VulkanDevice* device, RHISamplerDesc desc)
    : mParentDevice(device)
{
    VkSamplerAddressMode addressMode;
    switch (desc.Address)
    {
        case RHISamplerAddress::kWrap:   addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
        case RHISamplerAddress::kMirror: addressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
        case RHISamplerAddress::kClamp:  addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
        case RHISamplerAddress::kBorder: addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; break;
        default:                         addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
    }

    VkFilter filter;
    VkSamplerMipmapMode mipmapMode;
    switch (desc.Filter)
    {
        case RHISamplerFilter::kLinear:
            filter = VK_FILTER_LINEAR;
            mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case RHISamplerFilter::kNearest:
            filter = VK_FILTER_NEAREST;
            mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        default:
            filter = VK_FILTER_LINEAR;
            mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
    }

    VkSamplerCreateInfo samplerInfo = {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .magFilter               = filter,
        .minFilter               = filter,
        .mipmapMode             = desc.UseMips ? mipmapMode : VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU           = addressMode,
        .addressModeV           = addressMode,
        .addressModeW           = addressMode,
        .mipLodBias             = 0.0f,
        .anisotropyEnable       = VK_FALSE,
        .maxAnisotropy          = 1.0f,
        .compareEnable          = VK_FALSE,
        .compareOp              = VK_COMPARE_OP_ALWAYS,
        .minLod                 = 0.0f,
        .maxLod                 = desc.UseMips ? VK_LOD_CLAMP_NONE : 0.0f,
        .borderColor            = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkResult result = vkCreateSampler(mParentDevice->Device(), &samplerInfo, nullptr, &mSampler);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan sampler!");

    // Bindless
    mHandle.Index = mParentDevice->GetBindlessManager()->WriteSampler(this);

    SERAPH_WHATEVER("Created Vulkan sampler");
}

VulkanSampler::~VulkanSampler()
{
    mParentDevice->GetBindlessManager()->FreeSampler(mHandle.Index);
    if (mSampler) vkDestroySampler(mParentDevice->Device(), mSampler, nullptr);
}
