//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:04:09
//

#include "KGPU_VulkanDevice.h"

#include <KernelCore/KC_Assert.h>
#include <unordered_map>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace KGPU
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        if (!strcmp(pCallbackData->pMessageIdName, "VUID-VkShaderModuleCreateInfo-pCode-08737")) return VK_FALSE;

        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                KD_ERROR("Vulkan Error: %s", pCallbackData->pMessage);
                break;
            }
        }

        return VK_FALSE;
    }

    VulkanDevice::VulkanDevice(bool validationLayers)
    {
        VkResult result = volkInitialize();
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to initialize volk!");

        BuildInstance(validationLayers);
        BuildPhysicalDevice();
        BuildLogicalDevice();
        BuildAllocator();
        mBindlessManager = KC_NEW(VulkanBindlessManager, this);

        KD_INFO("Created Vulkan device!");
    }

    VulkanDevice::~VulkanDevice()
    {
        KC_DELETE(mBindlessManager);
        if (mAllocator) vmaDestroyAllocator(mAllocator);
        if (mDevice) vkDestroyDevice(mDevice, nullptr);
        if (mMessenger) vkDestroyDebugUtilsMessengerEXT(mInstance, mMessenger, nullptr);
        if (mInstance) vkDestroyInstance(mInstance, nullptr);
    }

    
    void VulkanDevice::BuildInstance(bool validationLayers)
    {
        uint32 instanceLayerCount = 1;
        const char* instanceLayers[] = { "VK_LAYER_KHRONOS_validation" };

        uint32 sdlExtensionCount = 2;
        auto sdlInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
        KC::Array<const char*> extensions(sdlInstanceExtensions, sdlInstanceExtensions + sdlExtensionCount);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
        appInfo.pApplicationName = "Seraph Application";
        appInfo.pEngineName = "Seraph Vulkan Renderer";

        VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {};
        messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.pfnUserCallback = VulkanDebugCallback;
        messengerInfo.pUserData = nullptr;

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledExtensionCount = extensions.size();
        instanceInfo.ppEnabledExtensionNames = extensions.data();
        if (validationLayers) {
            instanceInfo.enabledLayerCount = instanceLayerCount;
            instanceInfo.ppEnabledLayerNames = instanceLayers;
            instanceInfo.pNext = &messengerInfo;
        }

        VkResult result = vkCreateInstance(&instanceInfo, nullptr, &mInstance);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan instance!");

        volkLoadInstance(mInstance);

        // Setup debug messenger
        if (validationLayers) {        
            result = vkCreateDebugUtilsMessengerEXT(mInstance, &messengerInfo, nullptr, &mMessenger);
            KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan debug messenger!");
        }
    }

    void VulkanDevice::BuildPhysicalDevice()
    {
        uint gpuCount = 0;
        vkEnumeratePhysicalDevices(mInstance, &gpuCount, nullptr);
        KC::Array<VkPhysicalDevice> gpus(gpuCount);
        vkEnumeratePhysicalDevices(mInstance, &gpuCount, gpus.data());

        uint64 bestScore = 0;
        VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
        for (VkPhysicalDevice device : gpus) {
            uint64 score = CalculateDeviceScore(device);
            if (score > bestScore) {
                bestScore = score;
                bestDevice = device;
            }
        }

        mPhysicalDevice = bestDevice;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
        KD_INFO("Using GPU %s", properties.deviceName, bestScore);

        mBufferImageGranularity = properties.limits.bufferImageGranularity;
        mOptimalRowPitchAlignment = properties.limits.optimalBufferCopyRowPitchAlignment;
    }

    uint64 VulkanDevice::CalculateDeviceScore(VkPhysicalDevice device)
    {
        uint64 score = 0;
    
        // Query device properties and features
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
    
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
        VkPhysicalDeviceLimits limits = deviceProperties.limits;
    
        // Query supported extensions
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        KC::Array<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
        auto supportsExt = [&](const char* name) {
            for (auto& ext : availableExtensions) {
                if (strcmp(ext.extensionName, name) == 0) return true;
            }
            return false;
        };
    
        // Required baseline extensions
        const KC::Array<const char*> baselineExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
        };
    
        for (auto* required : baselineExtensions) {
            if (!supportsExt(required)) {
                KD_INFO("GPU %s is missing %s. Skipping", deviceProperties.deviceName, required);
                return 0;
            }
        }
    
        // Score weighting
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 100000000;
        if (deviceFeatures.depthClamp) score += 1000;
        if (deviceFeatures.geometryShader) score += 1000;
        if (deviceFeatures.multiDrawIndirect) score += 10000;
        if (deviceFeatures.textureCompressionBC) score += 10000;
        if (deviceFeatures.wideLines) score += 1000;
        if (deviceFeatures.samplerAnisotropy) score += 10000;
        if (deviceFeatures.pipelineStatisticsQuery) score += 10000;
    
        score += limits.maxDescriptorSetSamplers;
    
        return score;
    }
    
    void VulkanDevice::BuildLogicalDevice()
    {
        // Query supported extensions
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, nullptr);
        KC::Array<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
    
        auto supportsExt = [&](const char* name) {
            for (auto& ext : availableExtensions) {
                if (strcmp(ext.extensionName, name) == 0) return true;
            }
            return false;
        };
    
        KC::Array<const char*> enabledExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        };
    
        // Feature structs
        VkPhysicalDeviceFeatures2 deviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        VkPhysicalDeviceFeatures baseFeatures = {};
        baseFeatures.multiDrawIndirect = VK_TRUE;
        baseFeatures.drawIndirectFirstInstance = VK_TRUE;
        baseFeatures.tessellationShader = VK_TRUE;
        baseFeatures.depthClamp = VK_TRUE;
        baseFeatures.fillModeNonSolid = VK_TRUE;
        deviceFeatures2.features = baseFeatures;
    
        // Common features
        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
        descriptorIndexing.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        descriptorIndexing.runtimeDescriptorArray = VK_TRUE;
        descriptorIndexing.descriptorBindingVariableDescriptorCount = VK_TRUE;
        descriptorIndexing.descriptorBindingPartiallyBound = VK_TRUE;
        descriptorIndexing.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    
        VkPhysicalDeviceVulkan13Features vk13Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        vk13Features.dynamicRendering = VK_TRUE;
        vk13Features.shaderDemoteToHelperInvocation = VK_TRUE;
        vk13Features.synchronization2 = VK_TRUE;
    
        // Optional RT + Mesh Shader features
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accel = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
        accel.accelerationStructure = VK_TRUE;
        accel.descriptorBindingAccelerationStructureUpdateAfterBind = VK_TRUE;
    
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipeline = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
        rtPipeline.rayTracingPipeline = VK_TRUE;
        rtPipeline.rayTracingPipelineTraceRaysIndirect = VK_TRUE;
    
        VkPhysicalDeviceRayQueryFeaturesKHR rayQuery = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
        rayQuery.rayQuery = VK_TRUE;
    
        VkPhysicalDeviceBufferDeviceAddressFeatures bda = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES };
        bda.bufferDeviceAddress = VK_TRUE;
    
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShader = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
        meshShader.meshShader = VK_TRUE;
        meshShader.meshShaderQueries = VK_TRUE;
    
        // Build extension list + pNext chain
        void* nextChain = &vk13Features;
        vk13Features.pNext = &descriptorIndexing;
        void** chainTail = &descriptorIndexing.pNext;
    
        // Check RT extensions
        bool hasRT = supportsExt(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) &&
                     supportsExt(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) &&
                     supportsExt(VK_KHR_RAY_QUERY_EXTENSION_NAME) &&
                     supportsExt(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    
        if (hasRT) {
            enabledExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
            enabledExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        
            *chainTail = &accel;        chainTail = &accel.pNext;
            *chainTail = &rtPipeline;   chainTail = &rtPipeline.pNext;
            *chainTail = &rayQuery;     chainTail = &rayQuery.pNext;
            *chainTail = &bda;          chainTail = &bda.pNext;
        
            mSupportsRT = true;
        } else {
            mSupportsRT = false;
        }
    
        // Check Mesh Shader
        if (supportsExt(VK_EXT_MESH_SHADER_EXTENSION_NAME)) {
            enabledExtensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
            *chainTail = &meshShader;
            chainTail = &meshShader.pNext;
            mSupportsMS = true;
        } else {
            mSupportsMS = false;
        }
    
        *chainTail = nullptr; // terminate pNext
    
        // Queue family selection
        uint queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
        KC::Array<VkQueueFamilyProperties> families(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, families.data());
    
        uint graphicsIndex = UINT32_MAX;
        uint computeIndex = UINT32_MAX;
        uint transferIndex = UINT32_MAX;
    
        for (uint i = 0; i < queueFamilyCount; ++i) {
            const auto& flags = families[i].queueFlags;
            if ((flags & VK_QUEUE_GRAPHICS_BIT) && graphicsIndex == UINT32_MAX) graphicsIndex = i;
            if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && computeIndex == UINT32_MAX) computeIndex = i;
            if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT) && transferIndex == UINT32_MAX) transferIndex = i;
        }
    
        if (computeIndex == UINT32_MAX) computeIndex = graphicsIndex;
        if (transferIndex == UINT32_MAX) transferIndex = graphicsIndex;
    
        // Deduplicate queue families
        KC::Array<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint> uniqueFamilies = { graphicsIndex, computeIndex, transferIndex };
        float priority = 1.0f;
        for (uint idx : uniqueFamilies) {
            VkDeviceQueueCreateInfo queueInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queueInfo.queueFamilyIndex = idx;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &priority;
            queueCreateInfos.push_back(queueInfo);
        }
    
        // Create device
        VkDeviceCreateInfo deviceCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        deviceCreateInfo.pNext = nextChain;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint>(enabledExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
    
        VkResult result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create logical Vulkan device!");
    
        volkLoadDevice(mDevice);
    
        mGraphicsQueueFamilyIndex = graphicsIndex;
        mComputeQueueFamilyIndex = computeIndex;
        mTransferQueueFamilyIndex = transferIndex;
    }

    void VulkanDevice::BuildAllocator()
    {
        VmaVulkanFunctions functions = {};
        functions.vkAllocateMemory = vkAllocateMemory;
        functions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
        functions.vkBindBufferMemory = vkBindBufferMemory;
        functions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
        functions.vkBindImageMemory = vkBindImageMemory;
        functions.vkCmdCopyBuffer = vkCmdCopyBuffer;
        functions.vkCreateBuffer = vkCreateBuffer;
        functions.vkCreateImage = vkCreateImage;
        functions.vkDestroyBuffer = vkDestroyBuffer;
        functions.vkDestroyImage = vkDestroyImage;
        functions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        functions.vkFreeMemory = vkFreeMemory;
        functions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
        functions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        functions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
        functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        functions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
        functions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
        functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        functions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        functions.vkMapMemory = vkMapMemory;
        functions.vkUnmapMemory = vkUnmapMemory;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorInfo.instance = mInstance;
        allocatorInfo.physicalDevice = mPhysicalDevice;
        allocatorInfo.device = mDevice;
        allocatorInfo.pVulkanFunctions = &functions;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        VkResult result = vmaCreateAllocator(&allocatorInfo, &mAllocator);
        KD_ASSERT_EQ(result == VK_SUCCESS, "Failed to create VMA allocator!");
    }

    ISurface* VulkanDevice::CreateSurface(KOS::IWindow* window, ICommandQueue* graphicsQueue)
    {
        return KC_NEW(VulkanSurface, this, window, static_cast<VulkanCommandQueue*>(graphicsQueue));
    }

    ITexture* VulkanDevice::CreateTexture(TextureDesc desc)
    {
        return KC_NEW(VulkanTexture, this, desc);
    }

    ITextureView* VulkanDevice::CreateTextureView(TextureViewDesc desc)
    {
        return KC_NEW(VulkanTextureView, this, desc);
    }

    ICommandQueue* VulkanDevice::CreateCommandQueue(CommandQueueType type)
    {
        return KC_NEW(VulkanCommandQueue, this, type);
    }

    ISync* VulkanDevice::CreateSync(ISurface* surface, ICommandQueue* queue)
    {
    return KC_NEW(VulkanSync, this, static_cast<VulkanSurface*>(surface), static_cast<VulkanCommandQueue*>(queue));
    }

    IGraphicsPipeline* VulkanDevice::CreateGraphicsPipeline(GraphicsPipelineDesc desc)
    {
        return KC_NEW(VulkanGraphicsPipeline, this, desc);
    }

    IBuffer* VulkanDevice::CreateBuffer(BufferDesc desc)
    {
        return KC_NEW(VulkanBuffer, this, desc);
    }

    ISampler* VulkanDevice::CreateSampler(SamplerDesc desc)
    {
        return KC_NEW(VulkanSampler, this, desc);
    }

    IComputePipeline* VulkanDevice::CreateComputePipeline(ComputePipelineDesc desc)
    {
        return KC_NEW(VulkanComputePipeline, this, desc);
    }

    IBLAS* VulkanDevice::CreateBLAS(BLASDesc desc)
    {
        return KC_NEW(VulkanBLAS, this, desc);
    }

    ITLAS* VulkanDevice::CreateTLAS()
    {
        return KC_NEW(VulkanTLAS, this);
    }

    IBufferView* VulkanDevice::CreateBufferView(BufferViewDesc desc)
    {
        return KC_NEW(VulkanBufferView, this, desc);
    }
}
