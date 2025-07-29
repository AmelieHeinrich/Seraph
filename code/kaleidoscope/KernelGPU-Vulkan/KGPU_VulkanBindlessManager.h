//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-25 21:32:10
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelCore/KC_FreeList.h>

#include <volk.h>

namespace KGPU
{  
    class VulkanDevice;
    class VulkanTextureView;
    class VulkanSampler;
    class VulkanTLAS;
    class VulkanBufferView;

    class VulkanBindlessManager
    {
    public:
        VulkanBindlessManager(VulkanDevice* device);
        ~VulkanBindlessManager();

        VkDescriptorSetLayout GetLayout() const { return mLayout; }
        VkDescriptorSet GetSet() const { return mSet; }
        VkDescriptorPool GetPool() const { return mPool; }

        // ResourceDescriptorHeap[]
        uint WriteTextureSRV(VulkanTextureView* srv);
        uint WriteTextureUAV(VulkanTextureView* srv);
        uint WriteBufferCBV(VulkanBufferView* cbv);
        uint WriteBufferSRV(VulkanBufferView* cbv);
        uint WriteBufferUAV(VulkanBufferView* cbv);
        void FreeCBVSRVUAV(uint index);

        // SamplerDescriptorHeap[]
        uint WriteSampler(VulkanSampler* sampler);
        void FreeSampler(uint index);

        // AccelerationStructure
        uint WriteAS(VulkanTLAS* as);
        void FreeAS(uint index);

        // Other
        VkSampler GetGlobalSampler() { return mGlobalSampler; }
    private:
        VulkanDevice* mParentDevice;

        VkDescriptorSetLayout mLayout;
        VkDescriptorSet mSet;
        VkDescriptorPool mPool;

        VkSampler mGlobalSampler;

        KC::FreeList mResourceLUT;
        KC::FreeList mSamplerLUT;
        KC::FreeList mASLUT;
    };
}
