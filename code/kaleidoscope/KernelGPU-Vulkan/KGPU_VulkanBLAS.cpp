//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:37:46
//

#include "KGPU_VulkanBLAS.h"
#include "KGPU_VulkanDevice.h"

#undef max

namespace KGPU
{
    VulkanBLAS::VulkanBLAS(VulkanDevice* device, BLASDesc desc)
        : mParentDevice(device)
    {
        if (!mParentDevice->SupportsRaytracing())
            return;

        mDesc = desc;

        // Geometry
        mGeometry = {};
        mGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        mGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        mGeometry.flags = 0;
        mGeometry.geometry.triangles = {
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
            nullptr,
            VK_FORMAT_R32G32B32_SFLOAT,
            desc.VertexBuffer->GetAddress(),
            desc.VertexBuffer->GetDesc().Stride,
            desc.VertexCount,
            VK_INDEX_TYPE_UINT32,
            desc.IndexBuffer->GetAddress(),
            0
        };

        // Range info
        mRangeInfo.firstVertex = 0;
        mRangeInfo.transformOffset = 0;
        mRangeInfo.primitiveOffset = 0;
        mRangeInfo.primitiveCount = desc.IndexCount / 3;

        // Build info
        mBuildInfo = {};
        mBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        mBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        mBuildInfo.flags = desc.Static ? VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR : VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        mBuildInfo.geometryCount = 1;
        mBuildInfo.pGeometries = &mGeometry;
        mBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

        // Get sizes
        VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
        };

        vkGetAccelerationStructureBuildSizesKHR(
            mParentDevice->Device(),
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &mBuildInfo,
            &mRangeInfo.primitiveCount,
            &sizeInfo
        );

        mMemory = mParentDevice->CreateBuffer(BufferDesc(KOS::Align<uint>(sizeInfo.accelerationStructureSize, 256), 0, BufferUsage::kAccelerationStructure | BufferUsage::kShaderWrite));

        // Create!
        VkAccelerationStructureCreateInfoKHR asInfo = {
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
            nullptr,
            0,
            static_cast<VulkanBuffer*>(mMemory)->GetBuffer(),
            0,
            sizeInfo.accelerationStructureSize,
            VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            0
        };

        VkResult result = vkCreateAccelerationStructureKHR(mParentDevice->Device(), &asInfo, nullptr, &mHandle);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan BLAS!");

        // Create scratch buffer
        uint64 scratchSize = std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize);
        mScratch = mParentDevice->CreateBuffer(BufferDesc(KOS::Align<uint>(scratchSize, 256), 0, BufferUsage::kShaderWrite));

        // Update build info
        mBuildInfo.dstAccelerationStructure = mHandle;
        mBuildInfo.scratchData.deviceAddress = mScratch->GetAddress();

        KD_WHATEVER("Created Vulkan BLAS");
    }
    
    VulkanBLAS::~VulkanBLAS() 
    {
        if (!mParentDevice->SupportsRaytracing())
            return;

        KC_DELETE(mMemory);
        KC_DELETE(mScratch);
        if (mHandle) vkDestroyAccelerationStructureKHR(mParentDevice->Device(), mHandle, nullptr);
    }
    
    uint64 VulkanBLAS::GetAddress()
    {
        if (!mParentDevice->SupportsRaytracing())
            return 0;

        VkAccelerationStructureDeviceAddressInfoKHR address = {};
        address.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        address.accelerationStructure = mHandle;
        
        return vkGetAccelerationStructureDeviceAddressKHR(mParentDevice->Device(), &address);
    }
}
