//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 11:22:38
//

#pragma once

#include <RHI/Surface.hpp>
#include <Volk/volk.h>

class VulkanDevice;

class VulkanSurface : public IRHISurface
{
public:
    VulkanSurface(IRHIDevice* device, Window* window);
    ~VulkanSurface();

private:
    VulkanDevice* mParentDevice;

    VkSurfaceKHR mSurface;
};
