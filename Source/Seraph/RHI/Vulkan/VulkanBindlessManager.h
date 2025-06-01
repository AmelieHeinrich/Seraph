//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 16:26:13
//

#pragma once

#include <Core/Context.h>

#include <Volk/volk.h>

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
private:
    VulkanDevice* mParentDevice;

    VkDescriptorSetLayout mLayout;
    VkDescriptorSet mSet;
    VkDescriptorPool mPool;

    Array<bool> mResourceLUT;
    Array<bool> mSamplerLUT;
    Array<bool> mASLUT;
};
