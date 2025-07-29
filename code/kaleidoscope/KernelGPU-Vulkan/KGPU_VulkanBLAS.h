//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:36:36
//

#pragma once

#include <KernelGPU/KGPU_BLAS.h>

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanBLAS : public IBLAS
    {
    public:
        VulkanBLAS(VulkanDevice* device, BLASDesc desc);
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
}
