//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 19:45:44
//

#pragma once

#include <RHI/MeshPipeline.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanMeshPipeline : public IRHIMeshPipeline
{
public:
    VulkanMeshPipeline(VulkanDevice* device, RHIMeshPipelineDesc desc);
    ~VulkanMeshPipeline();

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
