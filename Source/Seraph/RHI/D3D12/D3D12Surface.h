//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:46:25
//

#pragma once

#include <RHI/Surface.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12Surface : public IRHISurface
{
public:
    D3D12Surface(D3D12Device* device, Window* window);
    ~D3D12Surface();
};
