//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:43:49
//

#pragma once

#include <KernelGPU/KGPU_ComputePipeline.h>

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanComputePipeline : public IComputePipeline
    {
    public:
        VulkanComputePipeline(VulkanDevice* device, ComputePipelineDesc desc);
        ~VulkanComputePipeline();

        VkPipeline GetPipeline() const { return mPipeline; }
        VkPipelineLayout GetLayout() const { return mLayout; }
        
    private:
        VulkanDevice* mParentDevice;
        
        VkPipeline mPipeline;
        VkPipelineLayout mLayout;
    };
}
