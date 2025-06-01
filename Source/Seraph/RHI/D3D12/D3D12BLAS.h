//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:08:35
//

#pragma once

#include <RHI/BLAS.h>
#include <RHI/Buffer.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12BLAS : public IRHIBLAS
{
public:
    D3D12BLAS(D3D12Device* device, RHIBLASDesc desc);
    ~D3D12BLAS();

    uint64 GetAddress() override;

private:
    friend class D3D12CommandBuffer;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
    D3D12_RAYTRACING_GEOMETRY_DESC mGeometry;
};
