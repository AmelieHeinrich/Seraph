//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:12:13
//

#include "KGPU_VulkanGraphicsPipeline.h"
#include "KGPU_VulkanDevice.h"
#include "KGPU_VulkanTexture.h"

namespace KGPU
{
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice* device, GraphicsPipelineDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        KC::Array<VkPipelineShaderStageCreateInfo> shaderStages;
        KC::Array<VkShaderModule> vkShaderModules;

        for (auto& [stage, module] : desc.Modules) {
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = module.Data.size();
            createInfo.pCode = reinterpret_cast<const uint*>(module.Data.data());

            VkShaderModule shaderModule;
            VkResult res = vkCreateShaderModule(mParentDevice->Device(), &createInfo, nullptr, &shaderModule);
            KD_ASSERT_EQ(res == VK_SUCCESS, "Failed to create shader module!");
            vkShaderModules.push_back(shaderModule);

            VkPipelineShaderStageCreateInfo stageInfo = {};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = ShaderStageToVk(stage);
            stageInfo.module = shaderModule;
            stageInfo.pName = module.Entry.c_str();
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
        KC::Array<VkPipelineColorBlendAttachmentState> blendAttachments;
        for (size_t i = 0; i < desc.RenderTargetFormats.size(); ++i) { 
            VkPipelineColorBlendAttachmentState attachment = {};
            attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            attachment.blendEnable = VK_FALSE;
            if (desc.EnableBlend) {
                attachment.blendEnable = VK_TRUE;
                attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attachment.colorBlendOp = VK_BLEND_OP_ADD;
                attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                attachment.alphaBlendOp = VK_BLEND_OP_ADD;
            }
            blendAttachments.push_back(attachment);
        }

        VkPipelineColorBlendStateCreateInfo colorBlend = {};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.attachmentCount = static_cast<uint>(blendAttachments.size());
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
        VkVertexInputBindingDescription vertexInputBindingDesc = {};
        vertexInputBindingDesc.binding = 0;
        vertexInputBindingDesc.stride = 0;
        vertexInputBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        KC::Array<VkVertexInputAttributeDescription> attributeDescriptions;

        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.vertexBindingDescriptionCount = vertexInputBindingDesc.stride == 0 ? 0 : 1;
        vertexInputState.pVertexBindingDescriptions = &vertexInputBindingDesc;
        vertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Multisample (single sample assumed)
        VkPipelineMultisampleStateCreateInfo multisample = {};
        multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Dynamic states (we assume viewport/scissor are dynamic)
        KC::Array<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Rendering info (for dynamic rendering)
        KC::Array<VkFormat> colorFormats;
        for (const auto& fmt : desc.RenderTargetFormats)
            colorFormats.push_back(VulkanTexture::RHIToVkFormat(fmt));

        VkPipelineRenderingCreateInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = static_cast<uint>(colorFormats.size());
        renderingInfo.pColorAttachmentFormats = colorFormats.data();
        renderingInfo.depthAttachmentFormat = VulkanTexture::RHIToVkFormat(desc.DepthFormat);

        // Pipeline layout (placeholder - assume you have one created)
        VkPushConstantRange pushRange = {};
        pushRange.offset = 0;
        pushRange.size = desc.PushConstantSize;
        pushRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

        VkDescriptorSetLayout bindlessLayout = mParentDevice->GetBindlessManager()->GetLayout();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &bindlessLayout;
        pipelineLayoutInfo.pushConstantRangeCount = pushRange.size > 0 ? 1 : 0;
        pipelineLayoutInfo.pPushConstantRanges = &pushRange;

        VkResult result = vkCreatePipelineLayout(mParentDevice->Device(), &pipelineLayoutInfo, nullptr, &mLayout);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint>(shaderStages.size());
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
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan graphics pipeline!");

        for (VkShaderModule module : vkShaderModules) {
            vkDestroyShaderModule(mParentDevice->Device(), module, nullptr);
        }

        mDesc.Modules.clear();

        KD_WHATEVER("Created Vulkan graphics pipeline");
    }
    
    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
        if (mPipeline) vkDestroyPipeline(mParentDevice->Device(), mPipeline, nullptr);
        if (mLayout) vkDestroyPipelineLayout(mParentDevice->Device(), mLayout, nullptr);
    }

    VkCompareOp VulkanGraphicsPipeline::ToVkCompareOp(DepthOperation op)
    {
        switch (op)
        {
            case DepthOperation::kGreater:    return VK_COMPARE_OP_GREATER;
            case DepthOperation::kLess:       return VK_COMPARE_OP_LESS;
            case DepthOperation::kEqual:      return VK_COMPARE_OP_EQUAL;
            case DepthOperation::kLessEqual:  return VK_COMPARE_OP_LESS_OR_EQUAL;
            case DepthOperation::kNone:       return VK_COMPARE_OP_ALWAYS;
            default:                             return VK_COMPARE_OP_ALWAYS;
        }
    }
    
    VkShaderStageFlagBits VulkanGraphicsPipeline::ShaderStageToVk(ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage::kVertex: return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::kPixel: return VK_SHADER_STAGE_FRAGMENT_BIT;
            default: return VK_SHADER_STAGE_ALL;
        }
    }
    
    VkCullModeFlagBits VulkanGraphicsPipeline::ToVkCullMode(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::kNone: return VK_CULL_MODE_NONE;
            case CullMode::kFront: return VK_CULL_MODE_FRONT_BIT;
            case CullMode::kBack: return VK_CULL_MODE_BACK_BIT;
            default: return VK_CULL_MODE_NONE;
        }
    }
    
    VkPolygonMode VulkanGraphicsPipeline::ToVkFillMode(FillMode mode)
    {
        switch (mode)
        {
            case FillMode::kSolid: return VK_POLYGON_MODE_FILL;
            case FillMode::kWireframe: return VK_POLYGON_MODE_LINE;
            default: return VK_POLYGON_MODE_FILL;
        }
    }
}
