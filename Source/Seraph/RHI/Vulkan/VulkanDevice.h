//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:31:28
//

#pragma once

#include <RHI/Device.hpp>
#include <RHI/Surface.hpp>

#include <volk/volk.h>

class VulkanDevice : public IRHIDevice
{
public:
    VulkanDevice(bool validationLayers);
    ~VulkanDevice();

    IRHISurface* CreateSurface(Window* window) override;

public:
    VkInstance Instance() const { return mInstance; }
    VkPhysicalDevice GPU() const { return mPhysicalDevice; }
    VkDevice Device() const { return mDevice; }

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;

    uint32 mGraphicsQueueFamilyIndex;
    uint32 mComputeQueueFamilyIndex;
    uint32 mTransferQueueFamilyIndex;

    void BuildInstance(bool validationLayers);
    void BuildPhysicalDevice();
    void BuildLogicalDevice();

private:
    uint64 CalculateDeviceScore(VkPhysicalDevice device);
};
