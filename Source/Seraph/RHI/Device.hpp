//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:26:45
//

#pragma once

#include <Core/Context.h>

enum class RHIBackend
{
    kVulkan,
    kD3D12
};

class IRHIDevice
{
public:
    ~IRHIDevice() = default;

    static IRHIDevice* CreateDevice(RHIBackend backend, bool validationLayers);

protected:
    IRHIDevice() = default;
};
