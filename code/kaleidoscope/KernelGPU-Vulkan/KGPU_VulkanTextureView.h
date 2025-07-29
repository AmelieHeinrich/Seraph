//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:01:26
//

#pragma once

#include <KernelGPU/KGPU_TextureView.h>

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanTextureView : public ITextureView
    {
    public:
        VulkanTextureView(VulkanDevice* device, TextureViewDesc viewDesc);
        ~VulkanTextureView();

        VkImageView GetView() const { return mImageView; }
    private:
        VkImageViewType RHIToVkImageViewType(TextureViewDimension dimension, TextureViewType type);

    private:
        VulkanDevice* mParentDevice;

        VkImageView mImageView = VK_NULL_HANDLE;
        VkDescriptorSet mImGuiSet = VK_NULL_HANDLE;
    };
}
