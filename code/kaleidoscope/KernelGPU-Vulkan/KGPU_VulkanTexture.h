//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:55:44
//

#pragma once

#include <KernelGPU/KGPU_Texture.h>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanTexture : public ITexture
    {
    public:
        VulkanTexture(TextureDesc desc);
        VulkanTexture(VulkanDevice* device, TextureDesc desc);
        ~VulkanTexture();

        void SetName(const KC::String& name) override;

        VmaAllocation Allocation() const { return mAllocation; }
        VmaAllocationInfo AllocationInfo() const { return mAllocationInfo; }
        VkImage Image() const { return mImage; }

    public:
        static VkFormat RHIToVkFormat(KGPU::TextureFormat format);

    private:
        friend class VulkanSurface;

        VulkanDevice* mParentDevice;

        VmaAllocation mAllocation;
        VmaAllocationInfo mAllocationInfo;

        VkImage mImage;
    };
}
