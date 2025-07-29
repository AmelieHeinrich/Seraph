//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:19:38
//

#pragma once

#include <KernelGPU/KGPU_Surface.h>

#include <KernelOS/KOS_Window.h>
#include <volk.h>

#include "KGPU_VulkanTexture.h"
#include "KGPU_VulkanTextureView.h"

namespace KGPU
{  
    class VulkanDevice;
    class VulkanCommandQueue;

    class VulkanSurface : public ISurface
    {
    public:
        VulkanSurface(VulkanDevice* device, KOS::IWindow* window, VulkanCommandQueue* commandQueue);
        ~VulkanSurface();

        VkSwapchainKHR GetSwapchain() const { return mSwapchain; }
    private:
        VulkanDevice* mParentDevice;

        VkSurfaceKHR mSurface;
        VkSwapchainKHR mSwapchain;
    };
}
