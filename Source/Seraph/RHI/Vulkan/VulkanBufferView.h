//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 12:07:13
//

#pragma once

#include <RHI/BufferView.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanBufferView : public IRHIBufferView
{
public:
    VulkanBufferView(VulkanDevice* device, RHIBufferViewDesc desc);
    ~VulkanBufferView();

private:
    VulkanDevice* mParentDevice;
};
