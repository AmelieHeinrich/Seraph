//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:08:14
//

#pragma once

#include <RHI/CommandBuffer.h>

#include "VulkanCommandQueue.h"

class VulkanCommandBuffer : public IRHICommandBuffer
{
public:
    VulkanCommandBuffer(VulkanDevice* device, VkCommandPool pool, bool singleTime);
    ~VulkanCommandBuffer();

private:
    VulkanDevice* mParentDevice;
    VkCommandPool mParentPool;
    
    VkCommandBuffer mCmdBuffer;
};
