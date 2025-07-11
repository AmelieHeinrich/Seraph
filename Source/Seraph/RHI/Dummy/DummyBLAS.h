//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:08:35
//

#pragma once

#include <RHI/BLAS.h>
#include <RHI/Buffer.h>

class DummyDevice;

class DummyBLAS : public IRHIBLAS
{
public:
    DummyBLAS(DummyDevice* device, RHIBLASDesc desc);
    ~DummyBLAS();

    uint64 GetAddress() override;
};
