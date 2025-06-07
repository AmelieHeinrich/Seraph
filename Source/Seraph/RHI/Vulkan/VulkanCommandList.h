//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:08:14
//

#pragma once

#include <RHI/CommandList.h>

#include "VulkanCommandQueue.h"

class VulkanCommandList : public IRHICommandList
{
public:
    VulkanCommandList(VulkanDevice* device, VkCommandPool pool, bool singleTime);
    ~VulkanCommandList();

    void Reset() override;
    void Begin() override;
    void End() override;

    void BeginRendering(const RHIRenderBegin& begin);
    void EndRendering();

    void Barrier(const RHITextureBarrier& barrier) override;
    void Barrier(const RHIBufferBarrier& barrier) override;
    void Barrier(const RHIMemoryBarrier& barrier) override;
    void BarrierGroup(const RHIBarrierGroup& barrierGroup) override;
    
    void ClearColor(IRHITextureView* view, float r, float g, float b) override;

    void SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline) override;
    void SetViewport(float width, float height, float x, float y) override;
    void SetVertexBuffer(IRHIBuffer* buffer) override;
    void SetIndexBuffer(IRHIBuffer* buffer) override;
    void SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size) override;
    void SetComputePipeline(IRHIComputePipeline* pipeline) override;
    void SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size) override;

    void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) override;
    void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance) override;
    void Dispatch(uint x, uint y, uint z) override;

    void CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src) override;
    void CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src) override;
    void CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src) override;
    void CopyTextureToTexture(IRHITexture* dst, IRHITexture* src) override;
    void BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode) override;
    void BuildTLAS(IRHITLAS* blas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer) override;
    
    void PushMarker(const StringView& name) override;
    void PopMarker() override;

    void BeginImGui() override;
    void EndImGui() override;
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
