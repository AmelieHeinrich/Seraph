//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 20:04:55
//

#include "VulkanBLAS.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"

VulkanBLAS::VulkanBLAS(VulkanDevice* device, RHIBLASDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    // Geometry
    mGeometry = {};
    mGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    mGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    mGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
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

    mMemory = mParentDevice->CreateBuffer(RHIBufferDesc(Align<uint>(sizeInfo.accelerationStructureSize, 256), 0, RHIBufferUsage::kAccelerationStructure | RHIBufferUsage::kShaderWrite));

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
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan BLAS!");

    // Create scratch buffer
    uint64 scratchSize = std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize);
    mScratch = mParentDevice->CreateBuffer(RHIBufferDesc(Align<uint>(scratchSize, 256), 0, RHIBufferUsage::kShaderWrite));

    // Update build info
    mBuildInfo.dstAccelerationStructure = mHandle;
    mBuildInfo.scratchData.deviceAddress = mScratch->GetAddress();

    SERAPH_WHATEVER("Created Vulkan BLAS");
}

VulkanBLAS::~VulkanBLAS()
{
    delete mMemory;
    delete mScratch;
    if (mHandle) vkDestroyAccelerationStructureKHR(mParentDevice->Device(), mHandle, nullptr);
}

uint64 VulkanBLAS::GetAddress()
{
    VkAccelerationStructureDeviceAddressInfoKHR address = {};
    address.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    address.accelerationStructure = mHandle;

    return vkGetAccelerationStructureDeviceAddressKHR(mParentDevice->Device(), &address);
}
