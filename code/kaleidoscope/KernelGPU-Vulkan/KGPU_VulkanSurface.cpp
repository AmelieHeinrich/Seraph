//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:23:43
//

#include "KGPU_VulkanSurface.h"
#include "KGPU_VulkanDevice.h"
#include "KGPU_VulkanCommandQueue.h"
#include "KGPU_VulkanTexture.h"
#include "KGPU_VulkanTextureView.h"

#include <SDL3/SDL_vulkan.h>

namespace KGPU
{
    VulkanSurface::VulkanSurface(VulkanDevice* device, KOS::IWindow* window, VulkanCommandQueue* commandQueue)
        : mParentDevice(device)
    {
        int width, height;
        window->GetSize(width, height);
        
        KC::String title = window->GetTitle();
        title += " | Vulkan";
        window->SetTitle(title.c_str());

        bool succeed = SDL_Vulkan_CreateSurface((SDL_Window*)window->GetHandle(), mParentDevice->Instance(), nullptr, &mSurface);
        KD_ASSERT_EQ(succeed, "Failed to create Vulkan surface!");

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
        KC::Array<VkSurfaceFormatKHR> formats(formatCount);
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
        swapchainInfo.imageExtent.width = width;
        swapchainInfo.imageExtent.height = height;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.preTransform = capabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = presentMode;
        swapchainInfo.clipped = VK_TRUE;
        swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(mParentDevice->Device(), &swapchainInfo, nullptr, &mSwapchain);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan swapchain!");

        // 6. get the images
        uint32 maxImages;
        vkGetSwapchainImagesKHR(mParentDevice->Device(), mSwapchain, &maxImages, nullptr);
        KC::Array<VkImage> images(maxImages);
        vkGetSwapchainImagesKHR(mParentDevice->Device(), mSwapchain, &maxImages, images.data());

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            TextureDesc desc = {};
            desc.Reserved = true;
            desc.Width = width;
            desc.Height = height;
            desc.Depth = 1;
            desc.MipLevels = 1;
            desc.Usage = TextureUsage::kRenderTarget;
            desc.Format = mParentDevice->GetSurfaceFormat();
        
            VulkanTexture* texture = KC_NEW(VulkanTexture, desc);
            texture->mDesc = desc;
            texture->mImage = images[i];
            texture->mParentDevice = mParentDevice;
            texture->mLayouts[0] = ResourceLayout::kUndefined;

            mTextures[i] = texture;
            mTextureViews[i] = KC_NEW(VulkanTextureView, device, TextureViewDesc(texture, TextureViewType::kRenderTarget));
        }
    
        KD_WHATEVER("Created Vulkan surface");
    }
    
    VulkanSurface::~VulkanSurface()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            KC_DELETE(mTextureViews[i]);
            KC_DELETE(mTextures[i]);
        }
        vkDestroySwapchainKHR(mParentDevice->Device(), mSwapchain, nullptr);
        vkDestroySurfaceKHR(mParentDevice->Instance(), mSurface, nullptr);
    }
}
