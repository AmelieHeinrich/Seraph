//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:31:28
//

#pragma once

#include <RHI/Device.hpp>

class VulkanDevice : public IRHIDevice
{
public:
    VulkanDevice(bool validationLayers);
    ~VulkanDevice();
};
