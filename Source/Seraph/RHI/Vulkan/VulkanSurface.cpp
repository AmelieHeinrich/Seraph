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
    // Surface
    SDL_Window* rawWindow = window->GetWindow();

    bool succeed = SDL_Vulkan_CreateSurface(rawWindow, mParentDevice->Instance(), nullptr, &mSurface);
    ASSERT_EQ(succeed, "Failed to create Vulkan surface!");

    // Swapchain
    int windowWidth, windowHeight;
    SDL_GetWindowSize(rawWindow, &windowWidth, &windowHeight);

    // 1. capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mParentDevice->GPU(), mSurface, &capabilities);

    // 2. frame count
    uint imageCount = FRAMES_IN_FLIGHT;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;
    if (imageCount < capabilities.minImageCount)
        imageCount = capabilities.minImageCount;
    
    // 3. format
    VkSurfaceFormatKHR surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    
    uint formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mParentDevice->GPU(), mSurface, &formatCount, nullptr);
    Array<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mParentDevice->GPU(), mSurface, &formatCount, formats.data());
    for (VkSurfaceFormatKHR format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = format;
            break;
        }
    }
    if (surfaceFormat.format != VK_FORMAT_B8G8R8A8_UNORM) {
        surfaceFormat = formats[0];
    }

    // 4. present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    // 5. create!
    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = mSurface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = surfaceFormat.format;
    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent.width = windowWidth;
    swapchainInfo.imageExtent.height = windowHeight;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainInfo.preTransform = capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(mParentDevice->Device(), &swapchainInfo, nullptr, &mSwapchain);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan swapchain!");

    // 6. get the images
    uint32 maxImages;
    vkGetSwapchainImagesKHR(mParentDevice->Device(), mSwapchain, &maxImages, nullptr);
    Array<VkImage> images(maxImages);
    vkGetSwapchainImagesKHR(mParentDevice->Device(), mSwapchain, &maxImages, images.data());

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        // Texture
        RHITextureDesc desc = {};
        desc.Reserved = true;
        desc.Width = windowWidth;
        desc.Height = windowHeight;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.Usage = RHITextureUsage::kRenderTarget;
        desc.Format = RHITextureFormat::kB8G8R8A8_UNORM;
        
        VulkanTexture* texture = new VulkanTexture;
        texture->mDesc = desc;
        texture->mImage = images[i];
        texture->mParentDevice = mParentDevice;
        mTextures[i] = texture;

        // View
        RHITextureViewDesc viewDesc(mTextures[i], RHITextureViewType::kRenderTarget);
        mTextureViews[i] = mParentDevice->CreateTextureView(viewDesc);
    }
}

VulkanSurface::~VulkanSurface()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mTextureViews[i];
        delete mTextures[i];
    }
    vkDestroySwapchainKHR(mParentDevice->Device(), mSwapchain, nullptr);
    vkDestroySurfaceKHR(mParentDevice->Instance(), mSurface, nullptr);
}
