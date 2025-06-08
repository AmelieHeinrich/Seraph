//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:22:00
//

#include "VulkanCommandList.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "VulkanTextureView.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanComputePipeline.h"
#include "VulkanBLAS.h"
#include "VulkanTLAS.H"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <ImGui/imgui_impl_vulkan.h>

VulkanCommandList::VulkanCommandList(VulkanDevice* device, VkCommandPool pool, bool singleTime)
    : mParentDevice(device), mParentPool(pool), mSingleTime(singleTime)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    
    VkResult result = vkAllocateCommandBuffers(device->Device(), &allocateInfo, &mCmdBuffer);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to allocate command buffer!");

    SERAPH_WHATEVER("Created Vulkan command buffer");
}

VulkanCommandList::~VulkanCommandList()
{
    if (mCmdBuffer) vkFreeCommandBuffers(mParentDevice->Device(), mParentPool, 1, &mCmdBuffer);
}

void VulkanCommandList::Reset()
{
    vkResetCommandBuffer(mCmdBuffer, 0);
}

void VulkanCommandList::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = mSingleTime ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

    VkResult result = vkBeginCommandBuffer(mCmdBuffer, &beginInfo);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to begin command buffer!");
}

void VulkanCommandList::End()
{
    VkResult result = vkEndCommandBuffer(mCmdBuffer);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to end command buffer!");
}

void VulkanCommandList::BeginRendering(const RHIRenderBegin& begin)
{
    Array<VkRenderingAttachmentInfo> colorAttachments;
    colorAttachments.reserve(begin.RenderTargets.size());

    for (const auto& rt : begin.RenderTargets) {
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = static_cast<VulkanTextureView*>(rt.View)->GetView();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = rt.Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f }; // Customize if needed

        colorAttachments.push_back(colorAttachment);
    }

    VkRenderingAttachmentInfo depthAttachment = {};
    const bool hasDepth = begin.DepthTarget.View != nullptr;

    if (hasDepth) {
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = static_cast<VulkanTextureView*>(begin.DepthTarget.View)->GetView();
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = begin.DepthTarget.Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = { 1.0f, 0 };
    }

    VkRenderingInfo renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = { 0, 0 };
    renderingInfo.renderArea.extent = { begin.Width, begin.Height };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = static_cast<uint>(colorAttachments.size());
    renderingInfo.pColorAttachments = colorAttachments.data();
    renderingInfo.pDepthAttachment = hasDepth ? &depthAttachment : nullptr;

    vkCmdBeginRendering(mCmdBuffer, &renderingInfo);
}

void VulkanCommandList::EndRendering()
{
    vkCmdEndRendering(mCmdBuffer);
}

void VulkanCommandList::Barrier(const RHITextureBarrier& barrier)
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
        .oldLayout           = TranslateLayoutToVk(barrier.Texture->GetLayout()),
        .newLayout           = TranslateLayoutToVk(barrier.NewLayout),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = vkTexture->Image(),
        .subresourceRange    = range
    };
    barrier.Texture->SetLayout(barrier.NewLayout);

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

void VulkanCommandList::Barrier(const RHIBufferBarrier& barrier)
{
    RHIBufferDesc desc = barrier.Buffer->GetDesc();
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(barrier.Buffer);

    const VkBufferMemoryBarrier2 bufferBarrier = {
        .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext               = nullptr,
        .srcStageMask        = TranslatePipelineStageToVk(barrier.SourceStage),
        .srcAccessMask       = TranslateAccessFlagsToVk(barrier.SourceAccess),
        .dstStageMask        = TranslatePipelineStageToVk(barrier.DestStage),
        .dstAccessMask       = TranslateAccessFlagsToVk(barrier.DestAccess),
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer              = vkBuffer->GetBuffer(),
        .offset              = 0,
        .size                = VK_WHOLE_SIZE
    };

    const VkDependencyInfo dependencyInfo = {
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = 0,
        .memoryBarrierCount       = 0,
        .pMemoryBarriers          = nullptr,
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers    = &bufferBarrier,
        .imageMemoryBarrierCount  = 0,
        .pImageMemoryBarriers     = nullptr
    };

    vkCmdPipelineBarrier2(mCmdBuffer, &dependencyInfo);
}

void VulkanCommandList::Barrier(const RHIMemoryBarrier& barrier)
{
    VkMemoryBarrier2 memoryBarrier = {
        .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .pNext         = nullptr,
        .srcStageMask  = TranslatePipelineStageToVk(barrier.SourceStage),
        .srcAccessMask = TranslateAccessFlagsToVk(barrier.SourceAccess),
        .dstStageMask  = TranslatePipelineStageToVk(barrier.DestStage),
        .dstAccessMask = TranslateAccessFlagsToVk(barrier.DestAccess)
    };

    const VkDependencyInfo dependencyInfo = {
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = 0,
        .memoryBarrierCount       = 1,
        .pMemoryBarriers          = &memoryBarrier,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers    = nullptr,
        .imageMemoryBarrierCount  = 0,
        .pImageMemoryBarriers     = nullptr
    };

    vkCmdPipelineBarrier2(mCmdBuffer, &dependencyInfo);
}

void VulkanCommandList::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{
    Array<VkImageMemoryBarrier2> imageBarriers;
    Array<VkBufferMemoryBarrier2> bufferBarriers;
    Array<VkMemoryBarrier2> memoryBarriers;
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
            .oldLayout           = TranslateLayoutToVk(barrier.Texture->GetLayout()),
            .newLayout           = TranslateLayoutToVk(barrier.NewLayout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = vkTexture->Image(),
            .subresourceRange    = range
        };
        barrier.Texture->SetLayout(barrier.NewLayout);
    
        imageBarriers.push_back(imageBarrier);
    }
    for (RHIBufferBarrier barrier : barrierGroup.BufferBarriers) {
        RHIBufferDesc desc = barrier.Buffer->GetDesc();
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(barrier.Buffer);

        const VkBufferMemoryBarrier2 bufferBarrier = {
            .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext               = nullptr,
            .srcStageMask        = TranslatePipelineStageToVk(barrier.SourceStage),
            .srcAccessMask       = TranslateAccessFlagsToVk(barrier.SourceAccess),
            .dstStageMask        = TranslatePipelineStageToVk(barrier.DestStage),
            .dstAccessMask       = TranslateAccessFlagsToVk(barrier.DestAccess),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer              = vkBuffer->GetBuffer(),
            .offset              = 0,
            .size                = VK_WHOLE_SIZE
        };

        bufferBarriers.push_back(bufferBarrier);
    }
    for (RHIMemoryBarrier barrier : barrierGroup.MemoryBarriers) {
        VkMemoryBarrier2 memoryBarrier = {
            .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .pNext         = nullptr,
            .srcStageMask  = TranslatePipelineStageToVk(barrier.SourceStage),
            .srcAccessMask = TranslateAccessFlagsToVk(barrier.SourceAccess),
            .dstStageMask  = TranslatePipelineStageToVk(barrier.DestStage),
            .dstAccessMask = TranslateAccessFlagsToVk(barrier.DestAccess)
        };

        memoryBarriers.push_back(memoryBarrier);
    }
    
    const VkDependencyInfo dependencyInfo = {
        .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext                    = nullptr,
        .dependencyFlags          = 0,
        .memoryBarrierCount       = static_cast<uint>(memoryBarriers.size()),
        .pMemoryBarriers          = memoryBarriers.data(),
        .bufferMemoryBarrierCount = static_cast<uint>(bufferBarriers.size()),
        .pBufferMemoryBarriers    = bufferBarriers.data(),
        .imageMemoryBarrierCount  = static_cast<uint>(imageBarriers.size()),
        .pImageMemoryBarriers     = imageBarriers.data()
    };

    vkCmdPipelineBarrier2(mCmdBuffer, &dependencyInfo);
}

void VulkanCommandList::ClearColor(IRHITextureView* view, float r, float g, float b)
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

void VulkanCommandList::SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline)
{
    VkDescriptorSet set = mParentDevice->GetBindlessManager()->GetSet();
    VulkanGraphicsPipeline* vkPipeline = static_cast<VulkanGraphicsPipeline*>(pipeline);

    vkCmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetPipeline());
    vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetLayout(), 0, 1, &set, 0, nullptr);
}

void VulkanCommandList::SetViewport(float width, float height, float x, float y)
{
    VkViewport viewport = {};
    viewport.width = width;
    viewport.height = -height;          // Negate height to flip vertically
    viewport.x = x;
    viewport.y = y + height;            // Shift Y down by the original height
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissorRect = {};
    scissorRect.extent.width = static_cast<uint>(width);
    scissorRect.extent.height = static_cast<uint>(height);
    scissorRect.offset.x = static_cast<int32_t>(x);
    scissorRect.offset.y = static_cast<int32_t>(y);

    vkCmdSetViewport(mCmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(mCmdBuffer, 0, 1, &scissorRect);
}

void VulkanCommandList::SetVertexBuffer(IRHIBuffer* buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    VkBuffer buf = vkBuffer->GetBuffer();

    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(mCmdBuffer, 0, 1, &buf, offsets);
}

void VulkanCommandList::SetIndexBuffer(IRHIBuffer* buffer)
{
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

    vkCmdBindIndexBuffer(mCmdBuffer, vkBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandList::SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size)
{
    VulkanGraphicsPipeline* vkPipeline = static_cast<VulkanGraphicsPipeline*>(pipeline);

    vkCmdPushConstants(mCmdBuffer, vkPipeline->GetLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, size, data);
}

void VulkanCommandList::SetComputePipeline(IRHIComputePipeline* pipeline)
{
    VkDescriptorSet set = mParentDevice->GetBindlessManager()->GetSet();
    VulkanComputePipeline* vkPipeline = static_cast<VulkanComputePipeline*>(pipeline);

    vkCmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetPipeline());
    vkCmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vkPipeline->GetLayout(), 0, 1, &set, 0, nullptr);
}

void VulkanCommandList::SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size)
{
    VulkanComputePipeline* vkPipeline = static_cast<VulkanComputePipeline*>(pipeline);

    vkCmdPushConstants(mCmdBuffer, vkPipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
}

void VulkanCommandList::Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
{
    vkCmdDraw(mCmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandList::DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
{
    vkCmdDrawIndexed(mCmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandList::Dispatch(uint x, uint y, uint z)
{
    vkCmdDispatch(mCmdBuffer, x, y, z);
}

void VulkanCommandList::CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src)
{
    VulkanBuffer* vkDest = static_cast<VulkanBuffer*>(dest);
    VulkanBuffer* vkSrc = static_cast<VulkanBuffer*>(src);

    VkBufferCopy copyRegion = {};
    copyRegion.size = src->GetDesc().Size;

    vkCmdCopyBuffer(mCmdBuffer, vkSrc->GetBuffer(), vkDest->GetBuffer(), 1, &copyRegion);
}

void VulkanCommandList::CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src)
{
    RHITextureDesc textureDesc = dest->GetDesc();
    VkImage image = static_cast<VulkanTexture*>(dest)->Image();
    VkBuffer buffer = static_cast<VulkanBuffer*>(src)->GetBuffer();

    const bool isBlockCompressed = IRHITexture::IsBlockFormat(textureDesc.Format);
    const uint bytesPerUnit = IRHITexture::BytesPerPixel(textureDesc.Format); // bytes per pixel or per block
    VkDeviceSize bufferOffset = 0;
    Array<VkBufferImageCopy> regions;
    
    for (uint mip = 0; mip < textureDesc.MipLevels; mip++) {
        uint mipWidth = std::max(1u, textureDesc.Width >> mip);
        uint mipHeight = std::max(1u, textureDesc.Height >> mip);
        uint rowPitch = 0;
        uint rowLength = 0;
        uint imageHeight = 0;
        VkExtent3D imageExtent = {};
        
        if (isBlockCompressed) {
            // Calculate block count (in 4x4 blocks)
            uint blocksWide = (mipWidth + 3) / 4;
            uint blocksHigh = (mipHeight + 3) / 4;
            rowPitch = Align<uint>(blocksWide * bytesPerUnit, TEXTURE_ROW_PITCH_ALIGNMENT);
            rowLength = (rowPitch / bytesPerUnit) * 4;
            imageExtent = { std::max(1u, mipWidth), std::max(1u, mipHeight), 1 };
        } else {
            rowPitch = Align<uint>(mipWidth * bytesPerUnit, TEXTURE_ROW_PITCH_ALIGNMENT);
            rowLength = rowPitch / bytesPerUnit;
            imageExtent = { mipWidth, mipHeight, 1 };
        }

        VkBufferImageCopy copyRegion = {};
        copyRegion.bufferOffset = bufferOffset;
        copyRegion.bufferRowLength = rowLength; // in texels, not bytes
        copyRegion.bufferImageHeight = 0;       // tightly packed between rows
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = mip;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = { 0, 0, 0 };
        copyRegion.imageExtent = imageExtent;
        regions.push_back(copyRegion);

        uint mipHeightInBlocks = isBlockCompressed ? (mipHeight + 3) / 4 : mipHeight;
        bufferOffset += rowPitch * mipHeightInBlocks;
    }

    vkCmdCopyBufferToImage(
        mCmdBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        regions.size(),
        regions.data()
    );
}

void VulkanCommandList::CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src)
{
    RHITextureDesc textureDesc = src->GetDesc();
    VkImage image = static_cast<VulkanTexture*>(src)->Image();
    VkBuffer buffer = static_cast<VulkanBuffer*>(dest)->GetBuffer();

    VkDeviceSize bufferOffset = 0;
    Array<VkBufferImageCopy> regions;

    for (uint mip = 0; mip < textureDesc.MipLevels; ++mip) {
        uint width = std::max(1u, textureDesc.Width >> mip);
        uint height = std::max(1u, textureDesc.Height >> mip);

        VkDeviceSize rowPitch;
        if (IRHITexture::IsBlockFormat(textureDesc.Format)) {
            uint blockWidth = (width + 3) / 4;
            rowPitch = Align<uint>(blockWidth * 16, 4); // Vulkan only requires row alignment to 4 bytes
        } else {
            uint bytesPerPixel = IRHITexture::BytesPerPixel(textureDesc.Format);
            rowPitch = Align<uint>(width * bytesPerPixel, 4);
        }

        VkBufferImageCopy region = {};
        region.bufferOffset = bufferOffset;
        region.bufferRowLength = 0; // Tightly packed
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mip;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        regions.push_back(region);

        VkDeviceSize heightInBlocks = IRHITexture::IsBlockFormat(textureDesc.Format)
            ? ((height + 3) / 4)
            : height;

        bufferOffset += rowPitch * heightInBlocks;
    }

    vkCmdCopyImageToBuffer(
        mCmdBuffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer,
        static_cast<uint>(regions.size()),
        regions.data()
    );
}

void VulkanCommandList::CopyTextureToTexture(IRHITexture* dst, IRHITexture* src)
{
    RHITextureDesc srcDesc = src->GetDesc();
    RHITextureDesc dstDesc = dst->GetDesc();

    VkImage srcImage = static_cast<VulkanTexture*>(src)->Image();
    VkImage dstImage = static_cast<VulkanTexture*>(dst)->Image();

    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcSubresource.baseArrayLayer = 0;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.srcOffset = { 0, 0, 0 };

    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.mipLevel = 0;
    copyRegion.dstSubresource.baseArrayLayer = 0;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.dstOffset = { 0, 0, 0 };

    copyRegion.extent.width = std::min(srcDesc.Width, dstDesc.Width);
    copyRegion.extent.height = std::min(srcDesc.Height, dstDesc.Height);
    copyRegion.extent.depth = 1; // assuming 2D textures

    vkCmdCopyImage(
        mCmdBuffer,
        srcImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &copyRegion
    );
}

void VulkanCommandList::BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode)
{
    VulkanBLAS* vkBlas = static_cast<VulkanBLAS*>(blas);

    const VkAccelerationStructureBuildRangeInfoKHR* range = &vkBlas->mRangeInfo;

    switch (mode) {
        case RHIASBuildMode::kRebuild: {
            vkBlas->mBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
            vkBlas->mBuildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
            vkBlas->mBuildInfo.dstAccelerationStructure = vkBlas->mHandle;
            vkBlas->mBuildInfo.scratchData.deviceAddress = vkBlas->mScratch->GetAddress();
            break;
        }
        case RHIASBuildMode::kRefit: {
            vkBlas->mBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
            vkBlas->mBuildInfo.srcAccelerationStructure = vkBlas->mHandle;
            vkBlas->mBuildInfo.dstAccelerationStructure = vkBlas->mHandle;
            vkBlas->mBuildInfo.scratchData.deviceAddress = vkBlas->mScratch->GetAddress();
            break;
        }
    }

    vkCmdBuildAccelerationStructuresKHR(mCmdBuffer, 1, &vkBlas->mBuildInfo, &range);
}

void VulkanCommandList::BuildTLAS(IRHITLAS* blas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer)
{
    VulkanTLAS* vkTlas = static_cast<VulkanTLAS*>(blas);
    vkTlas->mGeometry.geometry.instances.data.deviceAddress = buffer->GetAddress();
    vkTlas->mRangeInfo.primitiveCount = instanceCount;

    const VkAccelerationStructureBuildRangeInfoKHR* range = &vkTlas->mRangeInfo;
    switch (mode) {
        case RHIASBuildMode::kRebuild: {
            vkTlas->mBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
            vkTlas->mBuildInfo.srcAccelerationStructure = vkTlas->mHandle;
            vkTlas->mBuildInfo.dstAccelerationStructure = vkTlas->mHandle;
            break;
        }
        case RHIASBuildMode::kRefit: {
            vkTlas->mBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
            vkTlas->mBuildInfo.srcAccelerationStructure = vkTlas->mHandle;
            vkTlas->mBuildInfo.dstAccelerationStructure = vkTlas->mHandle;
            break;
        }
    }

    vkCmdBuildAccelerationStructuresKHR(mCmdBuffer, 1, &vkTlas->mBuildInfo, &range);
}

void VulkanCommandList::PushMarker(const StringView& name)
{
    VkDebugUtilsLabelEXT marker = {};
    marker.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    marker.pLabelName = name.data();

    vkCmdBeginDebugUtilsLabelEXT(mCmdBuffer, &marker);
}

void VulkanCommandList::PopMarker()
{
    vkCmdEndDebugUtilsLabelEXT(mCmdBuffer);
}

void VulkanCommandList::BeginImGui()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void VulkanCommandList::EndImGui()
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), mCmdBuffer);
}

VkPipelineStageFlags2 VulkanCommandList::TranslatePipelineStageToVk(RHIPipelineStage stage)
{
    VkPipelineStageFlags2 flags = 0;

    if (Any(stage & RHIPipelineStage::kTopOfPipe))            flags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    if (Any(stage & RHIPipelineStage::kDrawIndirect))         flags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    if (Any(stage & RHIPipelineStage::kVertexInput))          flags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    if (Any(stage & RHIPipelineStage::kIndexInput))           flags |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
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
    if (Any(stage & RHIPipelineStage::kAccelStructureWrite))  flags |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

    return flags;
}

VkAccessFlags2 VulkanCommandList::TranslateAccessFlagsToVk(RHIResourceAccess access)
{
    VkAccessFlags2 flags = 0;

    if (Any(access & RHIResourceAccess::kIndirectCommandRead))          flags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
    if (Any(access & RHIResourceAccess::kVertexBufferRead))             flags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
    if (Any(access & RHIResourceAccess::kIndexBufferRead))              flags |= VK_ACCESS_2_INDEX_READ_BIT;
    if (Any(access & RHIResourceAccess::kConstantBufferRead))           flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
    if (Any(access & RHIResourceAccess::kShaderRead))                   flags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    if (Any(access & RHIResourceAccess::kShaderWrite))                  flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kColorAttachmentRead))          flags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    if (Any(access & RHIResourceAccess::kColorAttachmentWrite))         flags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kDepthStencilRead))             flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    if (Any(access & RHIResourceAccess::kDepthStencilWrite))            flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kTransferRead))                 flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
    if (Any(access & RHIResourceAccess::kTransferWrite))                flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kHostRead))                     flags |= VK_ACCESS_2_HOST_READ_BIT;
    if (Any(access & RHIResourceAccess::kHostWrite))                    flags |= VK_ACCESS_2_HOST_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kMemoryRead))                   flags |= VK_ACCESS_2_MEMORY_READ_BIT;
    if (Any(access & RHIResourceAccess::kMemoryWrite))                  flags |= VK_ACCESS_2_MEMORY_WRITE_BIT;
    if (Any(access & RHIResourceAccess::kAccelerationStructureRead))    flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    if (Any(access & RHIResourceAccess::kAccelerationStructureWrite))   flags |= VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;

    return flags;
}

VkImageLayout VulkanCommandList::TranslateLayoutToVk(RHIResourceLayout layout)
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
