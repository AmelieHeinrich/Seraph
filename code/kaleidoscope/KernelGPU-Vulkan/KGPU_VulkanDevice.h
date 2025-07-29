//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:01:47
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

#include "KGPU_VulkanSurface.h"
#include "KGPU_VulkanTexture.h"
#include "KGPU_VulkanTextureView.h"
#include "KGPU_VulkanCommandQueue.h"
#include "KGPU_VulkanSync.h"
#include "KGPU_VulkanGraphicsPipeline.h"
#include "KGPU_VulkanBuffer.h"
#include "KGPU_VulkanSampler.h"
#include "KGPU_VulkanComputePipeline.h"
#include "KGPU_VulkanBLAS.h"
#include "KGPU_VulkanTLAS.h"
#include "KGPU_VulkanBufferView.h"
#include "KGPU_VulkanBindlessManager.h"

#include <vk_mem_alloc.h>
#include <volk.h>

namespace KGPU
{
    class VulkanDevice : public IDevice
    {
    public:
        VulkanDevice(bool validationLayers);
        ~VulkanDevice();

        ISurface* CreateSurface(KOS::IWindow* window, ICommandQueue* graphicsQueue) override;
        ITexture* CreateTexture(TextureDesc desc) override;
        ITextureView* CreateTextureView(TextureViewDesc desc) override;
        ICommandQueue* CreateCommandQueue(CommandQueueType type) override;
        ISync* CreateSync(ISurface* surface, ICommandQueue* queue) override;
        IGraphicsPipeline* CreateGraphicsPipeline(GraphicsPipelineDesc desc) override;
        IBuffer* CreateBuffer(BufferDesc desc) override;
        ISampler* CreateSampler(SamplerDesc desc) override;
        IComputePipeline* CreateComputePipeline(ComputePipelineDesc desc) override;
        IBLAS* CreateBLAS(BLASDesc desc) override;
        ITLAS* CreateTLAS() override;
        IBufferView* CreateBufferView(BufferViewDesc desc) override;

        Backend GetBackend() { return Backend::kVulkan; }
        bool SupportsRaytracing() { return mSupportsRT; }
        bool SupportsMeshShaders() { return mSupportsMS; }

        TextureFormat GetSurfaceFormat() override { return TextureFormat::kB8G8R8A8_UNORM; }
        uint64 GetOptimalRowPitchAlignment() override { return mOptimalRowPitchAlignment; }
        uint64 GetBufferImageGranularity() override { return mBufferImageGranularity; }
        KGPU::ShaderBytecodeType GetTargetBytecode() override { return KGPU::ShaderBytecodeType::kSPIRV; }

        VulkanBindlessManager* GetBindlessManager() { return mBindlessManager; }

    public:
        VkInstance Instance() const { return mInstance; }
        VkPhysicalDevice GPU() const { return mPhysicalDevice; }
        VkDevice Device() const { return mDevice; }
        VmaAllocator Allocator() const { return mAllocator; }
        
        uint32 GraphicsQueueFamilyIndex() const { return mGraphicsQueueFamilyIndex; }
        uint32 ComputeQueueFamilyIndex() const { return mComputeQueueFamilyIndex; }
        uint32 TransferQueueFamilyIndex() const { return mTransferQueueFamilyIndex; } 

    private:
        VkInstance mInstance;
        VkDebugUtilsMessengerEXT mMessenger;
        VkPhysicalDevice mPhysicalDevice;
        VkDevice mDevice;

        VmaAllocator mAllocator;

        bool mSupportsRT = false;
        bool mSupportsMS = false;

        VulkanBindlessManager* mBindlessManager;

        uint32 mGraphicsQueueFamilyIndex;
        uint32 mComputeQueueFamilyIndex;
        uint32 mTransferQueueFamilyIndex;
        uint32 mBufferImageGranularity;
        uint32 mOptimalRowPitchAlignment;

        void BuildInstance(bool validationLayers);
        void BuildPhysicalDevice();
        void BuildLogicalDevice();
        void BuildAllocator();

    private:
        uint64 CalculateDeviceScore(VkPhysicalDevice device);
    };
}
