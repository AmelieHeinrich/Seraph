//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 16:26:13
//

#pragma once

#include <Core/Context.h>

#include <Volk/volk.h>

constexpr uint64 MAX_BINDLESS_RESOURCES = 1000000;
constexpr uint64 MAX_BINDLESS_SAMPLERS = 2048;
constexpr uint64 MAX_BINDLESS_AS = 8;

class VulkanDevice;
class VulkanTextureView;
class VulkanSampler;
class VulkanTLAS;

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
