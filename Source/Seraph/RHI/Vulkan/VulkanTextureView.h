//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 15:35:50
//

#pragma once

#include <RHI/TextureView.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanTextureView : public IRHITextureView
{
public:
    VulkanTextureView(IRHIDevice* device, RHITextureViewDesc viewDesc);
    ~VulkanTextureView();

    VkImageView GetView() const { return mImageView; }

private:
    VkImageViewType RHIToVkImageViewType(RHITextureViewDimension dimension);

private:
    VulkanDevice* mParentDevice;

    VkImageView mImageView;
};
