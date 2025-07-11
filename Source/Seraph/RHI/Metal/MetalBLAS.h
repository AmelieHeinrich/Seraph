//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:08:35
//

#pragma once

#include <RHI/BLAS.h>
#include <RHI/Buffer.h>

class MetalDevice;

class MetalBLAS : public IRHIBLAS
{
public:
    MetalBLAS(MetalDevice* device, RHIBLASDesc desc);
    ~MetalBLAS();

    uint64 GetAddress() override;
};
