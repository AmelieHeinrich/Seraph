//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:31:28
//

#pragma once

#include <RHI/Device.h>
#include <RHI/Surface.h>
#include <RHI/Texture.h>
#include <RHI/TextureView.h>

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

class VulkanDevice : public IRHIDevice
{
public:
    VulkanDevice(bool validationLayers);
    ~VulkanDevice();

    IRHISurface* CreateSurface(Window* window) override;
    IRHITexture* CreateTexture(RHITextureDesc desc) override;
    IRHITextureView* CreateTextureView(RHITextureViewDesc desc) override;
    IRHICommandQueue* CreateCommandQueue(RHICommandQueueType type) override;
    IRHIF2FSync* CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue) override;

public:
    VkInstance Instance() const { return mInstance; }
    VkPhysicalDevice GPU() const { return mPhysicalDevice; }
    VkDevice Device() const { return mDevice; }
    VmaAllocator Allocator() const { return mAllocator; }

    uint32 GraphicsQueueFamilyIndex() const { return mGraphicsQueueFamilyIndex; }
    uint32 ComputeQueueFamilyIndex() const { return mComputeQueueFamilyIndex; }
    uint32 TransferQueueFamilyIndex() const { return mTransferQueueFamilyIndex; } 

    private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;

    VmaAllocator mAllocator;

    uint32 mGraphicsQueueFamilyIndex;
    uint32 mComputeQueueFamilyIndex;
    uint32 mTransferQueueFamilyIndex;

    void BuildInstance(bool validationLayers);
    void BuildPhysicalDevice();
    void BuildLogicalDevice();
    void BuildAllocator();

private:
    uint64 CalculateDeviceScore(VkPhysicalDevice device);
};
