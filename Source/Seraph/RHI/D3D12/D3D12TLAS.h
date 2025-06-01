//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:09:52
//

#pragma once

#include <RHI/TLAS.h>
#include <RHI/Buffer.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12TLAS : public IRHITLAS
{
public:
    D3D12TLAS(D3D12Device* device);
    ~D3D12TLAS();
};
