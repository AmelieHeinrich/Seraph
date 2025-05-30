//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 21:10:15
//

#include "VulkanGraphicsPipeline.h"

#include "VulkanGraphicsPipeline.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

VkCompareOp VulkanGraphicsPipeline::ToVkCompareOp(RHIDepthOperation op)
{
    switch (op)
    {
        case RHIDepthOperation::kGreater:    return VK_COMPARE_OP_GREATER;
        case RHIDepthOperation::kLess:       return VK_COMPARE_OP_LESS;
        case RHIDepthOperation::kEqual:      return VK_COMPARE_OP_EQUAL;
        case RHIDepthOperation::kLessEqual:  return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RHIDepthOperation::kNone:       return VK_COMPARE_OP_ALWAYS;
        default:                             return VK_COMPARE_OP_ALWAYS;
    }
}

VkShaderStageFlagBits VulkanGraphicsPipeline::ShaderStageToVk(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::kVertex: return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::kFragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::kGeometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::kTessellationEval: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::kTessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::kMesh: return VK_SHADER_STAGE_MESH_BIT_EXT;
        case ShaderStage::kAmplification: return VK_SHADER_STAGE_TASK_BIT_EXT;
        default: return VK_SHADER_STAGE_ALL;
    }
}

VkCullModeFlagBits VulkanGraphicsPipeline::ToVkCullMode(RHICullMode mode)
{
    switch (mode)
    {
        case RHICullMode::kNone: return VK_CULL_MODE_NONE;
        case RHICullMode::kFront: return VK_CULL_MODE_FRONT_BIT;
        case RHICullMode::kBack: return VK_CULL_MODE_BACK_BIT;
        default: return VK_CULL_MODE_NONE;
    }
}

VkPolygonMode VulkanGraphicsPipeline::ToVkFillMode(RHIFillMode mode)
{
    switch (mode)
    {
        case RHIFillMode::kSolid: return VK_POLYGON_MODE_FILL;
        case RHIFillMode::kWireframe: return VK_POLYGON_MODE_LINE;
        default: return VK_POLYGON_MODE_FILL;
    }
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, RHIGraphicsPipelineDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    Array<VkPipelineShaderStageCreateInfo> shaderStages;
    Array<VkShaderModule> vkShaderModules;

    for (auto& [stage, module] : desc.Bytecode)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = module.Bytecode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(module.Bytecode.data());

        VkShaderModule shaderModule;
        VkResult res = vkCreateShaderModule(mParentDevice->Device(), &createInfo, nullptr, &shaderModule);
        ASSERT_EQ(res == VK_SUCCESS, "Failed to create shader module!");
        vkShaderModules.push_back(shaderModule);

        VkPipelineShaderStageCreateInfo stageInfo = {};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = ShaderStageToVk(stage);
        stageInfo.module = shaderModule;
        stageInfo.pName = "main";
        shaderStages.push_back(stageInfo);
    }

    // Input assembly (basic)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = desc.LineTopology ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = desc.DepthClampEnabled ? VK_TRUE : VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = ToVkFillMode(desc.FillMode);
    rasterizer.cullMode = ToVkCullMode(desc.CullMode);
    rasterizer.frontFace = desc.CounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.lineWidth = 1.0f;

    // Color blend attachments
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;
    for (size_t i = 0; i < desc.RenderTargetFormats.size(); ++i)
    {
        VkPipelineColorBlendAttachmentState attachment = {};
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        attachment.blendEnable = VK_FALSE;
        blendAttachments.push_back(attachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlend = {};
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.attachmentCount = static_cast<uint32_t>(blendAttachments.size());
    colorBlend.pAttachments = blendAttachments.data();

    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.DepthEnabled;
    depthStencil.depthWriteEnable = desc.DepthWrite;
    depthStencil.depthCompareOp = ToVkCompareOp(desc.DepthOperation);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Input state
    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // Multisample (single sample assumed)
    VkPipelineMultisampleStateCreateInfo multisample = {};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Dynamic states (we assume viewport/scissor are dynamic)
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Rendering info (for dynamic rendering)
    std::vector<VkFormat> colorFormats;
    for (const auto& fmt : desc.RenderTargetFormats)
        colorFormats.push_back(VulkanTexture::RHIToVkFormat(fmt));

    VkPipelineRenderingCreateInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
    renderingInfo.pColorAttachmentFormats = colorFormats.data();
    renderingInfo.depthAttachmentFormat = desc.DepthEnabled ? VulkanTexture::RHIToVkFormat(desc.DepthFormat) : VK_FORMAT_UNDEFINED;

    // Pipeline layout (placeholder - assume you have one created)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkResult result = vkCreatePipelineLayout(mParentDevice->Device(), &pipelineLayoutInfo, nullptr, &mLayout);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pColorBlendState = &colorBlend;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mLayout;
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.pVertexInputState = &vertexInputState;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = 0;

    result = vkCreateGraphicsPipelines(mParentDevice->Device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan graphics pipeline!");

    for (VkShaderModule module : vkShaderModules) {
        vkDestroyShaderModule(mParentDevice->Device(), module, nullptr);
    }

    mDesc.Bytecode.clear();
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    if (mPipeline) vkDestroyPipeline(mParentDevice->Device(), mPipeline, nullptr);
    if (mLayout) vkDestroyPipelineLayout(mParentDevice->Device(), mLayout, nullptr);
}
