//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:37
//

#pragma once

#include <RHI/Device.hpp>

class D3D12Device : public IRHIDevice
{
public:
    D3D12Device(bool validationLayers);
    ~D3D12Device();
};
