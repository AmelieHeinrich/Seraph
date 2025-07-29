//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:26:51
//

#pragma once

#include <KernelGPU/KGPU_Sync.h>

#include "KGPU_VulkanSurface.h"
#include "KGPU_VulkanCommandQueue.h"

namespace KGPU
{
    class VulkanSync : public ISync
    {
    public:
        VulkanSync(VulkanDevice* device, VulkanSurface* surface, VulkanCommandQueue* queue);
        ~VulkanSync();

        uint BeginSynchronize() override;
        void EndSynchronize(ICommandList* submitBuffer) override;
        void PresentSurface() override;
    private:
        VulkanDevice* mParentDevice;
        VulkanSurface* mSurface;
        VulkanCommandQueue* mCommandQueue;

        VkSemaphore mImageAvailableSemaphore[FRAMES_IN_FLIGHT];
        VkSemaphore mImageRenderedSemaphore[FRAMES_IN_FLIGHT];
        VkFence mInFlightFences[FRAMES_IN_FLIGHT];
        uint32 mCurrentFrame = 0;
        uint32 mImageIndex = 0;
    };
}
