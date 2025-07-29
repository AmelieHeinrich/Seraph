//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:36:36
//

#pragma once

#include <KernelGPU/KGPU_BLAS.h>

#include <Agility/d3d12.h>

namespace KGPU
{
    class D3D12Device;

    class D3D12BLAS : public IBLAS
    {
    public:
        D3D12BLAS(D3D12Device* device, BLASDesc desc);
        ~D3D12BLAS();

        uint64 GetAddress() override;

    private:
        friend class D3D12CommandList;
        
        D3D12Device* mParentDevice;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS mInputs;
        D3D12_RAYTRACING_GEOMETRY_DESC mGeometry;
    };
}
