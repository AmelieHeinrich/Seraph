//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:32:21
//

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <set>

#include "VulkanDevice.h"
#include "VulkanSurface.h"
#include "VulkanTexture.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            SERAPH_ERROR("Vulkan Error: %s", pCallbackData->pMessage);
            break;
        }
    }

    return VK_FALSE;
}

VulkanDevice::VulkanDevice(bool validationLayers)
{
    VkResult result = volkInitialize();
    ASSERT_EQ(result == VK_SUCCESS, "Failed to initialize volk!");

    BuildInstance(validationLayers);
    BuildPhysicalDevice();
    BuildLogicalDevice();
    BuildAllocator();

    SERAPH_INFO("Created Vulkan device!");
}

VulkanDevice::~VulkanDevice()
{
    if (mAllocator) vmaDestroyAllocator(mAllocator);
    if (mDevice) vkDestroyDevice(mDevice, nullptr);
    if (mMessenger) vkDestroyDebugUtilsMessengerEXT(mInstance, mMessenger, nullptr);
    if (mInstance) vkDestroyInstance(mInstance, nullptr);
    
    volkFinalize();
}

IRHISurface* VulkanDevice::CreateSurface(Window* window)
{
    return (new VulkanSurface(this, window));
}

IRHITexture* VulkanDevice::CreateTexture(RHITextureDesc desc)
{
    return (new VulkanTexture(this, desc));
}

void VulkanDevice::BuildInstance(bool validationLayers)
{
    uint32 instanceLayerCount = 1;
    const char* instanceLayers[] = { "VK_LAYER_KHRONOS_validation" };

    uint32 sdlExtensionCount = 0;
    const char* const* sdlInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
    Array<const char*> extensions(sdlInstanceExtensions, sdlInstanceExtensions + sdlExtensionCount);
    if (validationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan instance!");

    volkLoadInstance(mInstance);

    // Setup debug messenger
    if (validationLayers) {        
        result = vkCreateDebugUtilsMessengerEXT(mInstance, &messengerInfo, nullptr, &mMessenger);
        ASSERT_EQ(result == VK_SUCCESS, "Failed to create Vulkan debug messenger!");
    }
}

void VulkanDevice::BuildPhysicalDevice()
{
    uint gpuCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &gpuCount, nullptr);
    Array<VkPhysicalDevice> gpus(gpuCount);
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
    SERAPH_INFO("Using GPU %s with score %llu", properties.deviceName, bestScore);
}

uint64 VulkanDevice::CalculateDeviceScore(VkPhysicalDevice device)
{
    uint64 score = 0;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);   
    
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    VkPhysicalDeviceLimits limits = deviceProperties.limits;

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
    // Required extensions
    const Array<const char*> requiredExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_MAINTENANCE3_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
        VK_EXT_MUTABLE_DESCRIPTOR_TYPE_EXTENSION_NAME,
        VK_KHR_RAY_QUERY_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
    };

    // Base features
    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceFeatures baseFeatures = {};
    baseFeatures.multiDrawIndirect = VK_TRUE;
    baseFeatures.drawIndirectFirstInstance = VK_TRUE;
    deviceFeatures2.features = baseFeatures;

    // Feature chain
    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
    descriptorIndexing.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    descriptorIndexing.runtimeDescriptorArray = VK_TRUE;
    descriptorIndexing.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptorIndexing.descriptorBindingPartiallyBound = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
    dynamicRendering.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructure = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
    accelerationStructure.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipeline = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
    rayTracingPipeline.rayTracingPipeline = VK_TRUE;
    rayTracingPipeline.rayTracingPipelineTraceRaysIndirect = VK_TRUE;

    VkPhysicalDeviceRayQueryFeaturesKHR rayQuery = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
    rayQuery.rayQuery = VK_TRUE;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShader = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
    meshShader.meshShader = VK_TRUE;
    meshShader.meshShaderQueries = VK_TRUE;

    VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT mutableDescriptor = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MUTABLE_DESCRIPTOR_TYPE_FEATURES_EXT };
    mutableDescriptor.mutableDescriptorType = VK_TRUE;

    // Chain them
    deviceFeatures2.pNext = &descriptorIndexing;
    descriptorIndexing.pNext = &dynamicRendering;
    dynamicRendering.pNext = &accelerationStructure;
    accelerationStructure.pNext = &rayTracingPipeline;
    rayTracingPipeline.pNext = &rayQuery;
    rayQuery.pNext = &meshShader;
    meshShader.pNext = &mutableDescriptor;

    // Queue family selection
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
    Array<VkQueueFamilyProperties> families(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, families.data());

    uint32_t graphicsIndex = UINT32_MAX;
    uint32_t computeIndex = UINT32_MAX;
    uint32_t transferIndex = UINT32_MAX;

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        const auto& flags = families[i].queueFlags;

        if ((flags & VK_QUEUE_GRAPHICS_BIT) && graphicsIndex == UINT32_MAX)
            graphicsIndex = i;

        if ((flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && computeIndex == UINT32_MAX)
            computeIndex = i;

        if ((flags & VK_QUEUE_TRANSFER_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT) && transferIndex == UINT32_MAX)
            transferIndex = i;
    }

    // Fallback if no dedicated compute/transfer
    if (computeIndex == UINT32_MAX) computeIndex = graphicsIndex;
    if (transferIndex == UINT32_MAX) transferIndex = graphicsIndex;

    // Prepare queue create infos (deduplicate by index)
    Array<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { graphicsIndex, computeIndex, transferIndex };
    float priority = 1.0f;

    for (uint32_t index : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = index;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
        queueCreateInfos.push_back(queueInfo);
    }

    // Create device
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &deviceFeatures2;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

    VkResult result = vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create logical Vulkan device!");

    volkLoadDevice(mDevice);

    // Store queue family indices if needed
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

    VkResult result = vmaCreateAllocator(&allocatorInfo, &mAllocator);
    ASSERT_EQ(result == VK_SUCCESS, "Failed to create VMA allocator!");
}
