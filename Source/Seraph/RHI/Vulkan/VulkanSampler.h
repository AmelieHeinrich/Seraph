//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 14:40:40
//

#pragma once

#include <RHI/Sampler.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanSampler : public IRHISampler
{
public:
    VulkanSampler(VulkanDevice* device, RHISamplerDesc desc);
    ~VulkanSampler() override;

public:
    VkSampler GetSampler() { return mSampler; }

private:
    VulkanDevice* mParentDevice;

    VkSampler mSampler;
};
