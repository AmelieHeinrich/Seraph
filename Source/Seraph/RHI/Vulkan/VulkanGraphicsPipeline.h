//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 21:04:46
//

#pragma once

#include <RHI/GraphicsPipeline.h>

#include <Volk/volk.h>

class VulkanDevice;

class VulkanGraphicsPipeline : public IRHIGraphicsPipeline
{
public:
    VulkanGraphicsPipeline(VulkanDevice* device, RHIGraphicsPipelineDesc desc);
    ~VulkanGraphicsPipeline();

    VkPipeline GetPipeline() const { return mPipeline; }
    VkPipelineLayout GetLayout() const { return mLayout; }

private:
    VkCompareOp ToVkCompareOp(RHIDepthOperation op);
    VkCullModeFlagBits ToVkCullMode(RHICullMode mode);
    VkShaderStageFlagBits ShaderStageToVk(ShaderStage stage);
    VkPolygonMode ToVkFillMode(RHIFillMode mode);

private:
    VulkanDevice* mParentDevice;

    VkPipeline mPipeline;
    VkPipelineLayout mLayout;
};
