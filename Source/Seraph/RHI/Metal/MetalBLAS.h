//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:08:35
//

#pragma once

#include <RHI/BLAS.h>
#include <RHI/Buffer.h>

#include <MetalCPP/Metal/Metal.hpp>

class MetalDevice;

class MetalBLAS : public IRHIBLAS
{
public:
    MetalBLAS(MetalDevice* device, RHIBLASDesc desc);
    ~MetalBLAS();

    uint64 GetAddress() override;

public:
    MTL::AccelerationStructure* GetAccelerationStructure() { return mAS; }
    MTL::AccelerationStructureDescriptor* GetDescriptor() { return mDescriptor; }

private:
    MTL::AccelerationStructure* mAS;
    MTL::AccelerationStructureTriangleGeometryDescriptor* mGeometryDescriptor;
    MTL::AccelerationStructureDescriptor* mDescriptor;
};
