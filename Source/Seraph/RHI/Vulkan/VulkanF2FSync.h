//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:48:04
//

#pragma once

#include <RHI/F2FSync.h>

#include "VulkanSurface.h"
#include "VulkanCommandQueue.h"
#include "VulkanCommandBuffer.h"

class VulkanF2FSync : public IRHIF2FSync
{
public:
    VulkanF2FSync(VulkanDevice* device, VulkanSurface* surface, VulkanCommandQueue* queue);
    ~VulkanF2FSync();

    uint BeginSynchronize() override;
    void EndSynchronize(IRHICommandBuffer* submitBuffer) override;
    void PresentSurface() override;
private:
    VulkanDevice* mParentDevice;
    VulkanSurface* mSurface;
    VulkanCommandQueue* mCommandQueue;

    StaticArray<VkSemaphore, FRAMES_IN_FLIGHT> mImageAvailableSemaphore;
    StaticArray<VkSemaphore, FRAMES_IN_FLIGHT> mImageRenderedSemaphore;
    StaticArray<VkFence, FRAMES_IN_FLIGHT> mInFlightFences;
    uint32 mCurrentFrame = 0;
    uint32 mImageIndex = 0;
};
