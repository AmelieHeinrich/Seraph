//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 21:56:00
//

#pragma once

#include <RHI/TLAS.h>
#include <RHI/Buffer.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanTLAS : public IRHITLAS
{
public:
    VulkanTLAS(VulkanDevice* device);
    ~VulkanTLAS();

    VkAccelerationStructureKHR GetHandle() const { return mHandle; }
private:
    friend class VulkanCommandList;

    VulkanDevice* mParentDevice;

    VkAccelerationStructureKHR mHandle;
    VkAccelerationStructureBuildGeometryInfoKHR mBuildInfo;
    VkAccelerationStructureBuildRangeInfoKHR mRangeInfo;
    VkAccelerationStructureGeometryKHR mGeometry;
};
