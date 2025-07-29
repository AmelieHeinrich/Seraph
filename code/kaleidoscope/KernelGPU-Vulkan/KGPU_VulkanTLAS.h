//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:05:21
//

#pragma once

#include <KernelGPU/KGPU_TLAS.h>

#include <volk.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanTLAS : public ITLAS
    {
    public:
        VulkanTLAS(VulkanDevice* device);
        ~VulkanTLAS();
    
        uint64 Address() const { return mMemory->GetAddress(); }

        VkAccelerationStructureKHR GetHandle() { return mHandle; }
    private:
        friend class VulkanCommandList;

        VulkanDevice* mParentDevice;

        VkAccelerationStructureKHR mHandle;
        VkAccelerationStructureBuildGeometryInfoKHR mBuildInfo;
        VkAccelerationStructureBuildRangeInfoKHR mRangeInfo;
        VkAccelerationStructureGeometryKHR mGeometry;
    };
}
