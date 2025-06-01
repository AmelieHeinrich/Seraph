//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 19:14:03
//

#pragma once

#include <RHI/ComputePipeline.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanComputePipeline : public IRHIComputePipeline
{
public:
    VulkanComputePipeline(VulkanDevice* device, RHIComputePipelineDesc desc);
    ~VulkanComputePipeline();

    VkPipeline GetPipeline() const { return mPipeline; }
    VkPipelineLayout GetLayout() const { return mLayout; }

private:
    VulkanDevice* mParentDevice;

    VkPipeline mPipeline;
    VkPipelineLayout mLayout;
};
