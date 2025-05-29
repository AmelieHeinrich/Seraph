//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 11:22:38
//

#pragma once

#include <RHI/Surface.h>

#include <Volk/volk.h>

#include "VulkanTexture.h"
#include "VulkanTextureView.h"

class VulkanDevice;

class VulkanSurface : public IRHISurface
{
public:
    VulkanSurface(IRHIDevice* device, Window* window);
    ~VulkanSurface();

private:
    VulkanDevice* mParentDevice;

    VkSurfaceKHR mSurface;
    VkSwapchainKHR mSwapchain;

    StaticArray<IRHITexture*, FRAMES_IN_FLIGHT> mTextures;
    StaticArray<IRHITextureView*, FRAMES_IN_FLIGHT> mTextureViews;
};
