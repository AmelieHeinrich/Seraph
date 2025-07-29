//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:11:40
//

#pragma once

#include <KernelGPU/KGPU_GraphicsPipeline.h>

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanGraphicsPipeline : public IGraphicsPipeline
    {
    public:
        VulkanGraphicsPipeline(VulkanDevice* device, GraphicsPipelineDesc desc);
        ~VulkanGraphicsPipeline();

        VkPipeline GetPipeline() const { return mPipeline; }
        VkPipelineLayout GetLayout() const { return mLayout; }

    private:
        VkCompareOp ToVkCompareOp(DepthOperation op);
        VkCullModeFlagBits ToVkCullMode(CullMode mode);
        VkShaderStageFlagBits ShaderStageToVk(ShaderStage stage);
        VkPolygonMode ToVkFillMode(FillMode mode);

    private:
        VulkanDevice* mParentDevice;

        VkPipeline mPipeline;
        VkPipelineLayout mLayout;
    };
}
