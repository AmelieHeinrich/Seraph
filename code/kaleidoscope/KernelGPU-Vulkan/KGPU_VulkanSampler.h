//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:08:46
//

#pragma once

#include <KernelGPU/KGPU_Sampler.h>

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanSampler : public ISampler
    {
    public:
        VulkanSampler(VulkanDevice* device, SamplerDesc desc);
        ~VulkanSampler();

    public:
        VkSampler GetSampler() { return mSampler; }

    private:
        VulkanDevice* mParentDevice;

        VkSampler mSampler;
    };
}
