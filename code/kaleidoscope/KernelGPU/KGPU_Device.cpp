//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:27:19
//

#include "KGPU_Device.h"

#include <KernelCore/KC_Context.h>

#include <KernelGPU-Dummy/KGPU_DummyDevice.h>
#ifdef KD_WINDOWS
    #include <KernelGPU-D3D12/KGPU_D3D12Device.h>
    #include <KernelGPU-Vulkan/KGPU_VulkanDevice.h>
#endif

namespace KGPU
{
    IDevice* IDevice::Create(bool debug)
    {
#if KD_WINDOWS
        return KC_NEW(D3D12Device, debug);
#else
        return KC_NEW(DummyDevice, debug);
#endif
    }
}
