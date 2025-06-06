//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 19:16:21
//

#include "VulkanComputePipeline.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"

VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device, RHIComputePipelineDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    VkShaderModule computeModule;

    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = desc.ComputeBytecode.Bytecode.size();
    shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(desc.ComputeBytecode.Bytecode.data());

    VkResult result = vkCreateShaderModule(mParentDevice->Device(), &shaderCreateInfo, nullptr, &computeModule);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create shader module!");

    VkPushConstantRange pushRange = {};
    pushRange.offset = 0;
    pushRange.size = desc.PushConstantSize;
    pushRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayout bindlessLayout = mParentDevice->GetBindlessManager()->GetLayout();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &bindlessLayout;
    pipelineLayoutInfo.pushConstantRangeCount = pushRange.size > 0 ? 1 : 0;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;

    result = vkCreatePipelineLayout(mParentDevice->Device(), &pipelineLayoutInfo, nullptr, &mLayout);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan pipeline layout!");

    VkComputePipelineCreateInfo computePipeInfo = {};
    computePipeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipeInfo.layout = mLayout;
    computePipeInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipeInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computePipeInfo.stage.module = computeModule;
    computePipeInfo.stage.pName = "main";

    result = vkCreateComputePipelines(mParentDevice->Device(), VK_NULL_HANDLE, 1, &computePipeInfo, nullptr, &mPipeline);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan compute pipeline!");

    vkDestroyShaderModule(mParentDevice->Device(), computeModule, nullptr);

    SERAPH_WHATEVER("Created Vulkan compute pipeline");
}

VulkanComputePipeline::~VulkanComputePipeline()
{
    if (mPipeline) vkDestroyPipeline(mParentDevice->Device(), mPipeline, nullptr);
    if (mLayout) vkDestroyPipelineLayout(mParentDevice->Device(), mLayout, nullptr);
}
