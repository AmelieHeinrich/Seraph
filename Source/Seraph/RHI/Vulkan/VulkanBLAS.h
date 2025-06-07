//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 20:02:27
//

#pragma once

#include <RHI/BLAS.h>

#include <Vk/volk.h>

class VulkanDevice;

class VulkanBLAS : public IRHIBLAS
{
public:
    VulkanBLAS(VulkanDevice* device, RHIBLASDesc desc);
    ~VulkanBLAS();

    uint64 GetAddress() override;
private:
    friend class VulkanCommandList;

    VulkanDevice* mParentDevice;

    VkAccelerationStructureKHR mHandle;
    VkAccelerationStructureBuildGeometryInfoKHR mBuildInfo;
    VkAccelerationStructureGeometryKHR mGeometry;
    VkAccelerationStructureBuildRangeInfoKHR mRangeInfo;
};
