//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:06:13
//

#include "KGPU_VulkanTLAS.h"
#include "KGPU_VulkanDevice.h"

#undef max

namespace KGPU
{
    VulkanTLAS::VulkanTLAS(VulkanDevice* device)
        : mParentDevice(device)
    {
        if (!mParentDevice->SupportsRaytracing())
            return;

        mGeometry = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
            .pNext = nullptr,
            .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
            .geometry = {
                .instances = {
                    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                    .arrayOfPointers = VK_FALSE,
                    .data = {},
                }
            },
            .flags = VK_GEOMETRY_OPAQUE_BIT_KHR
        };

        mBuildInfo = {};
        mBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        mBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        mBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR; // if you want to allow refit
        mBuildInfo.geometryCount = 1;
        mBuildInfo.pGeometries = &mGeometry;
        mBuildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

        mRangeInfo = {
            .primitiveCount = MAX_TLAS_INSTANCES,
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0
        };

        VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR
        };

        vkGetAccelerationStructureBuildSizesKHR(
            mParentDevice->Device(),
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &mBuildInfo,
            &mRangeInfo.primitiveCount,
            &sizeInfo
        );

        mMemory = mParentDevice->CreateBuffer(BufferDesc(KOS::Align<uint>(sizeInfo.accelerationStructureSize, 256), 0, BufferUsage::kAccelerationStructure | BufferUsage::kShaderWrite));

        VkAccelerationStructureCreateInfoKHR asInfo = {
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
            nullptr,
            0,
            static_cast<VulkanBuffer*>(mMemory)->GetBuffer(),
            0,
            sizeInfo.accelerationStructureSize,
            VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
            0
        };

        VkResult result = vkCreateAccelerationStructureKHR(mParentDevice->Device(), &asInfo, nullptr, &mHandle);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan TLAS!");

        // Create scratch buffer
        uint64 scratchSize = std::max(sizeInfo.buildScratchSize, sizeInfo.updateScratchSize);
        mScratch = mParentDevice->CreateBuffer(BufferDesc(KOS::Align<uint>(scratchSize, 256), 0, BufferUsage::kShaderWrite));

        // Update build info
        mBuildInfo.dstAccelerationStructure = mHandle;
        mBuildInfo.scratchData.deviceAddress = mScratch->GetAddress();

        // Descriptor!
        mBindless = mParentDevice->GetBindlessManager()->WriteAS(this);
        
        KD_WHATEVER("Created Vulkan TLAS");
    }
    
    VulkanTLAS::~VulkanTLAS()
    {
        if (!mParentDevice->SupportsRaytracing())
            return;

        mParentDevice->GetBindlessManager()->FreeAS(mBindless.Index);
        KC_DELETE(mMemory);
        KC_DELETE(mScratch);
        if (mHandle) vkDestroyAccelerationStructureKHR(mParentDevice->Device(), mHandle, nullptr);
    }
}
