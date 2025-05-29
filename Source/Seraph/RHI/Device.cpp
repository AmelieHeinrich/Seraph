//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:34:26
//

#include "Device.h"

#include "Vulkan/VulkanDevice.h"
#include "D3D12/D3D12Device.h"

IRHIDevice* IRHIDevice::CreateDevice(RHIBackend backend, bool validationLayers)
{
    switch (backend)
    {
        case RHIBackend::kD3D12: return new D3D12Device(validationLayers);
        case RHIBackend::kVulkan: return new VulkanDevice(validationLayers);
    }
    return nullptr;
}
