//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:26:18
//

#pragma once

#include <KernelGPU/KGPU_Buffer.h>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanBuffer : public IBuffer
    {
    public:
        VulkanBuffer(VulkanDevice* device, BufferDesc desc);
        ~VulkanBuffer();

        void SetName(const KC::String& name) override;

        void* Map() override;
        void Unmap(uint start = 0, uint end = 0) override;

        uint64 GetAddress() override;

    public:
        VmaAllocation GetAllocation() const { return mAllocation; }
        VmaAllocationInfo GetAllocationInfo() const { return mAllocationInfo; }
        VkBuffer GetBuffer() const { return mBuffer; }

    private:
        VulkanDevice* mParentDevice;
        
        VmaAllocation mAllocation;
        VmaAllocationInfo mAllocationInfo;
        
        VkBuffer mBuffer;
    };
}
