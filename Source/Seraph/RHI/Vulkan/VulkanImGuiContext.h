//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 22:53:47
//

#pragma once

#include <RHI/ImGuiContext.h>
#include <Core/Window.h>

#include <Vk/volk.h>

class VulkanDevice;
class VulkanCommandQueue;

class VulkanImGuiContext : public IRHIImGuiContext
{
public:
    VulkanImGuiContext(VulkanDevice* device, VulkanCommandQueue* queue, Window* window);
    ~VulkanImGuiContext();
};
