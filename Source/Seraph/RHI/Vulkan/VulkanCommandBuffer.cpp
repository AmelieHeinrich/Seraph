//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:22:00
//

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, bool singleTime)
    : mParentDevice(device), mParentPool(pool), mSingleTime(singleTime)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    VkResult result = vkAllocateCommandBuffers(device->Device(), &allocateInfo, &mCmdBuffer);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to allocate command buffer!");
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    if (mCmdBuffer) vkFreeCommandBuffers(mParentDevice->Device(), mParentPool, 1, &mCmdBuffer);
}

void VulkanCommandBuffer::Reset()
{
    vkResetCommandBuffer(mCmdBuffer, 0);
}

void VulkanCommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = mSingleTime ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

    VkResult result = vkBeginCommandBuffer(mCmdBuffer, &beginInfo);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to begin command buffer!");
}

void VulkanCommandBuffer::End()
{
    VkResult result = vkEndCommandBuffer(mCmdBuffer);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to end command buffer!");
}

void VulkanCommandBuffer::Barrier(const RHITextureBarrier& barrier)
{
    RHITextureDesc desc = barrier.Texture->GetDesc();
    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(barrier.Texture);

    VkImageSubresourceRange range = {
        .aspectMask     = static_cast<VkImageAspectFlags>(Any(desc.Usage & RHITextureUsage::kDepthTarget) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
        .baseMipLevel   = barrier.BaseMipLevel,
        .levelCount     = barrier.LevelCount,
        .baseArrayLayer = barrier.ArrayLayer,
        .layerCount     = barrier.LayerCount
    };

    const VkImageMemoryBarrier2 imageBarrier = {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext               = nullptr,
        .srcStageMask        = TranslatePipelineStageToVk(barrier.SourceStage),
        .srcAccessMask       = TranslateAccessFlagsToVk(barrier.SourceAccess),
        .dstStageMask        = TranslatePipelineStageToVk(barrier.DestStage),
        .dstAccessMask       = TranslateAccessFlagsToVk(barrier.DestAccess),
        .oldLayout           = TranslateLayoutToVk(barrier.OldLayout),
        .newLayout           = TranslateLayoutToVk(barrier.NewLayout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = vkTexture->Image(),
        .subresourceRange    = range
    };

    const VkDependencyInfo dependencyInfo = {
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = 0,
        .memoryBarrierCount       = 0,
        .pMemoryBarriers          = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers    = nullptr,
        .imageMemoryBarrierCount  = 1,
        .pImageMemoryBarriers     = &imageBarrier
    };

    vkCmdPipelineBarrier2(mCmdBuffer, &dependencyInfo);
}

void VulkanCommandBuffer::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{
    Array<VkImageMemoryBarrier2> imageBarriers;
    for (RHITextureBarrier barrier : barrierGroup.TextureBarriers) {
        RHITextureDesc desc = barrier.Texture->GetDesc();
        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(barrier.Texture);

        VkImageSubresourceRange range = {
            .aspectMask     = static_cast<VkImageAspectFlags>(Any(desc.Usage & RHITextureUsage::kDepthTarget) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
            .baseMipLevel   = barrier.BaseMipLevel,
            .levelCount     = barrier.LevelCount,
            .baseArrayLayer = barrier.ArrayLayer,
            .layerCount     = barrier.LayerCount
        };

        const VkImageMemoryBarrier2 imageBarrier {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext               = nullptr,
            .srcStageMask        = TranslatePipelineStageToVk(barrier.SourceStage),
            .srcAccessMask       = TranslateAccessFlagsToVk(barrier.SourceAccess),
            .dstStageMask        = TranslatePipelineStageToVk(barrier.DestStage),
            .dstAccessMask       = TranslateAccessFlagsToVk(barrier.DestAccess),
            .oldLayout           = TranslateLayoutToVk(barrier.OldLayout),
            .newLayout           = TranslateLayoutToVk(barrier.NewLayout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = vkTexture->Image(),
            .subresourceRange    = range
        };
    
        imageBarriers.push_back(imageBarrier);
    }
    
    const VkDependencyInfo dependencyInfo = {
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = 0,
        .memoryBarrierCount       = 0,
        .pMemoryBarriers          = nullptr,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers    = nullptr,
        .imageMemoryBarrierCount  = static_cast<uint>(imageBarriers.size()),
        .pImageMemoryBarriers     = imageBarriers.data()
    };

    vkCmdPipelineBarrier2(mCmdBuffer, &dependencyInfo);
}

void VulkanCommandBuffer::ClearColor(IRHITextureView* view, float r, float g, float b)
{
    VulkanTexture* texture = static_cast<VulkanTexture*>(view->GetDesc().Texture);

    VkImageSubresourceRange range = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = view->GetDesc().ViewMip == VIEW_ALL_MIPS ? 0 : static_cast<uint32>(view->GetDesc().ViewMip),
        .levelCount     = view->GetDesc().ViewMip == VIEW_ALL_MIPS ? VK_REMAINING_MIP_LEVELS : 1,
        .baseArrayLayer = static_cast<uint32>(view->GetDesc().ArrayLayer),
        .layerCount     = 1
    };

    VkClearColorValue clearColor = {};
    clearColor.float32[0] = r;
    clearColor.float32[1] = g;
    clearColor.float32[2] = b;
    clearColor.float32[3] = 1.0f;

    vkCmdClearColorImage(mCmdBuffer, texture->Image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
}

VkPipelineStageFlags2 VulkanCommandBuffer::TranslatePipelineStageToVk(RHIPipelineStage stage)
{
    VkPipelineStageFlags2 flags = 0;

    if (Any(stage & RHIPipelineStage::kTopOfPipe))            flags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    if (Any(stage & RHIPipelineStage::kDrawIndirect))         flags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    if (Any(stage & RHIPipelineStage::kVertexInput))          flags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    if (Any(stage & RHIPipelineStage::kVertexShader))         flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    if (Any(stage & RHIPipelineStage::kHullShader))           flags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
    if (Any(stage & RHIPipelineStage::kDomainShader))         flags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
    if (Any(stage & RHIPipelineStage::kGeometryShader))       flags |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
    if (Any(stage & RHIPipelineStage::kPixelShader))          flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    if (Any(stage & RHIPipelineStage::kComputeShader))        flags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (Any(stage & RHIPipelineStage::kRayTracingShader))     flags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
    if (Any(stage & RHIPipelineStage::kEarlyFragmentTests))   flags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
    if (Any(stage & RHIPipelineStage::kLateFragmentTests))    flags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    if (Any(stage & RHIPipelineStage::kColorAttachmentOutput))flags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (Any(stage & RHIPipelineStage::kResolve))              flags |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
    if (Any(stage & RHIPipelineStage::kCopy))                 flags |= VK_PIPELINE_STAGE_2_COPY_BIT;
    if (Any(stage & RHIPipelineStage::kBottomOfPipe))         flags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    if (Any(stage & RHIPipelineStage::kAllGraphics))          flags |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    if (Any(stage & RHIPipelineStage::kAllCommands))          flags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    return flags;
}

VkAccessFlags2 VulkanCommandBuffer::TranslateAccessFlagsToVk(RHIResourceAccess access)
{
    VkAccessFlags2 flags = 0;

    if (Any(access & RHIResourceAccess::kIndirectCommandRead))  flags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    if (Any(access & RHIResourceAccess::kVertexBufferRead))     flags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    if (Any(access & RHIResourceAccess::kIndexBufferRead))      flags |= VK_ACCESS_2_INDEX_READ_BIT;
    if (Any(access & RHIResourceAccess::kConstantBufferRead))   flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
    if (Any(access & RHIResourceAccess::kShaderRead))           flags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    if (Any(access & RHIResourceAccess::kShaderWrite))          flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kColorAttachmentRead))  flags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    if (Any(access & RHIResourceAccess::kColorAttachmentWrite)) flags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kDepthStencilRead))     flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (Any(access & RHIResourceAccess::kDepthStencilWrite))    flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kTransferRead))         flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
    if (Any(access & RHIResourceAccess::kTransferWrite))        flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kHostRead))             flags |= VK_ACCESS_2_HOST_READ_BIT;
    if (Any(access & RHIResourceAccess::kHostWrite))            flags |= VK_ACCESS_2_HOST_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kMemoryRead))           flags |= VK_ACCESS_2_MEMORY_READ_BIT;
    if (Any(access & RHIResourceAccess::kMemoryWrite))          flags |= VK_ACCESS_2_MEMORY_WRITE_BIT;

    return flags;
}

VkImageLayout VulkanCommandBuffer::TranslateLayoutToVk(RHIResourceLayout layout)
{
    switch (layout) {
        case RHIResourceLayout::kUndefined:             return VK_IMAGE_LAYOUT_UNDEFINED;
        case RHIResourceLayout::kGeneral:               return VK_IMAGE_LAYOUT_GENERAL;
        case RHIResourceLayout::kReadOnly:              return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case RHIResourceLayout::kColorAttachment:       return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RHIResourceLayout::kDepthStencilReadOnly:  return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case RHIResourceLayout::kDepthStencilWrite:     return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case RHIResourceLayout::kTransferSrc:           return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case RHIResourceLayout::kTransferDst:           return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case RHIResourceLayout::kPresent:               return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:                                        return VK_IMAGE_LAYOUT_UNDEFINED;
    }
    return VK_IMAGE_LAYOUT_UNDEFINED;
}
