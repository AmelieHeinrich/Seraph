//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:34:26
//

#include "Device.h"

#ifdef SERAPH_VULKAN
    #include "Vulkan/VulkanDevice.h"
#endif
#ifdef SERAPH_D3D12
    #include "D3D12/D3D12Device.h"
#endif
#include "Dummy/DummyDevice.h"

IRHIDevice* IRHIDevice::CreateDevice(RHIBackend backend, bool validationLayers)
{
    switch (backend)
    {
#ifdef SERAPH_D3D12
        case RHIBackend::kD3D12: return new D3D12Device(validationLayers);
#endif
#ifdef SERAPH_VULKAN
        case RHIBackend::kVulkan: return new VulkanDevice(validationLayers);
#endif
        case RHIBackend::kDummy: return new DummyDevice(validationLayers);
        default: return new DummyDevice(validationLayers);
    }
    return nullptr;
}
