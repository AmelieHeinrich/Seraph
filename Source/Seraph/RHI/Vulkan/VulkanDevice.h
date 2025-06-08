//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:31:28
//

#pragma once

#include <RHI/Device.h>
#include <RHI/Surface.h>
#include <RHI/Texture.h>
#include <RHI/TextureView.h>

#include <vma/vk_mem_alloc.h>
#include <Vk/volk.h>

#include "VulkanBindlessManager.h"

class VulkanDevice : public IRHIDevice
{
public:
    VulkanDevice(bool validationLayers);
    ~VulkanDevice();

    IRHISurface* CreateSurface(Window* window, IRHICommandQueue* graphicsQueue) override;
    IRHITexture* CreateTexture(RHITextureDesc desc) override;
    IRHITextureView* CreateTextureView(RHITextureViewDesc desc) override;
    IRHICommandQueue* CreateCommandQueue(RHICommandQueueType type) override;
    IRHIF2FSync* CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue) override;
    IRHIGraphicsPipeline* CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc) override;
    IRHIBuffer* CreateBuffer(RHIBufferDesc desc) override;
    IRHISampler* CreateSampler(RHISamplerDesc desc) override;
    IRHIComputePipeline* CreateComputePipeline(RHIComputePipelineDesc desc) override;
    IRHIMeshPipeline* CreateMeshPipeline(RHIMeshPipelineDesc desc) override;
    IRHIBLAS* CreateBLAS(RHIBLASDesc desc) override;
    IRHITLAS* CreateTLAS() override;
    IRHIBufferView* CreateBufferView(RHIBufferViewDesc desc) override;
    IRHIImGuiContext* CreateImGuiContext(IRHICommandQueue* mainQueue, Window* window) override;

    RHITextureFormat GetSurfaceFormat() override { return RHITextureFormat::kB8G8R8A8_UNORM; }
    uint64 GetOptimalRowPitchAlignment() override { return mOptimalRowPitchAlignment; }
public:
    VkInstance Instance() const { return mInstance; }
    VkPhysicalDevice GPU() const { return mPhysicalDevice; }
    VkDevice Device() const { return mDevice; }
    VmaAllocator Allocator() const { return mAllocator; }

    VulkanBindlessManager* GetBindlessManager() { return mBindlessManager; }

    uint32 GraphicsQueueFamilyIndex() const { return mGraphicsQueueFamilyIndex; }
    uint32 ComputeQueueFamilyIndex() const { return mComputeQueueFamilyIndex; }
    uint32 TransferQueueFamilyIndex() const { return mTransferQueueFamilyIndex; } 

private:
    uint GetTexelBlockSize(RHITextureFormat format);

private:
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT mMessenger;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;

    VmaAllocator mAllocator;

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
