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

    void Barrier(const RHITextureBarrier& barrier) override;
    void BarrierGroup(const RHIBarrierGroup& barrierGroup) override;
    
    void ClearColor(IRHITextureView* view, float r, float g, float b) override;

public:
    VkCommandBuffer GetCommandBuffer() { return mCmdBuffer; }

private:
    VkPipelineStageFlags2 TranslatePipelineStageToVk(RHIPipelineStage stage);
    VkAccessFlags2 TranslateAccessFlagsToVk(RHIResourceAccess access);
    VkImageLayout TranslateLayoutToVk(RHIResourceLayout layout);

private:
    bool mSingleTime = false;

    VulkanDevice* mParentDevice;
    VkCommandPool mParentPool;
    
    VkCommandBuffer mCmdBuffer;
};
