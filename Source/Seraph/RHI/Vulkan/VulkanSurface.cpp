//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 11:23:37
//

#include "VulkanSurface.h"
#include "VulkanDevice.h"

#include <SDL3/SDL_vulkan.h>

VulkanSurface::VulkanSurface(IRHIDevice* device, Window* window)
    : mParentDevice(static_cast<VulkanDevice*>(device))
{
    SDL_Window* rawWindow = window->GetWindow();

    bool succeed = SDL_Vulkan_CreateSurface(rawWindow, mParentDevice->Instance(), nullptr, &mSurface);
    ASSERT_EQ(succeed, "Failed to create Vulkan surface!");
}

VulkanSurface::~VulkanSurface()
{
    vkDestroySurfaceKHR(mParentDevice->Instance(), mSurface, nullptr);
}
