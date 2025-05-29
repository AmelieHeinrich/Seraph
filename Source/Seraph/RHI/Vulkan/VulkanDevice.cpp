//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:32:21
//

#include "VulkanDevice.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

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

    SERAPH_INFO("Created Vulkan device!");
}

VulkanDevice::~VulkanDevice()
{
    if (mMessenger) vkDestroyDebugUtilsMessengerEXT(mInstance, mMessenger, nullptr);
    vkDestroyInstance(mInstance, nullptr);
    volkFinalize();
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
