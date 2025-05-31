//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:08:14
//

#pragma once

#include <RHI/CommandBuffer.h>

#include "VulkanCommandQueue.h"

class VulkanCommandBuffer : public IRHICommandBuffer
{
public:
    VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, bool singleTime);
    ~VulkanCommandBuffer();

    void Reset() override;
    void Begin() override;
    void End() override;

    void BeginRendering(const RHIRenderBegin& begin);
    void EndRendering();

    void Barrier(const RHITextureBarrier& barrier) override;
    void Barrier(const RHIBufferBarrier& barrier) override;
    void BarrierGroup(const RHIBarrierGroup& barrierGroup) override;
    
    void ClearColor(IRHITextureView* view, float r, float g, float b) override;

    void SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline) override;
    void SetViewport(float width, float height, float x, float y) override;
    void SetVertexBuffer(IRHIBuffer* buffer) override;
    void SetIndexBuffer(IRHIBuffer* buffer) override;
    void SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size) override;

    void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) override;
    void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance) override;

    void CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src) override;
    void CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src) override;
public:
    VkCommandBuffer GetCommandBuffer() { return mCmdBuffer; }

private:
    static VkPipelineStageFlags2 TranslatePipelineStageToVk(RHIPipelineStage stage);
    static VkAccessFlags2 TranslateAccessFlagsToVk(RHIResourceAccess access);
    static VkImageLayout TranslateLayoutToVk(RHIResourceLayout layout);

private:
    bool mSingleTime = false;

    VulkanDevice* mParentDevice;
    VkCommandPool mParentPool;
    
    VkCommandBuffer mCmdBuffer;
};
