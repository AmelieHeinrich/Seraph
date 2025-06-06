//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:10:22
//

#include "D3D12TLAS.h"
#include "D3D12Device.h"

#undef max

D3D12TLAS::D3D12TLAS(D3D12Device* device)
    : mParentDevice(device)
{
    mInputs = {};
    mInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    mInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    mInputs.NumDescs = MAX_TLAS_INSTANCES;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&mInputs, &prebuildInfo);

    mMemory = device->CreateBuffer(RHIBufferDesc(Align<uint>(prebuildInfo.ResultDataMaxSizeInBytes, 256), 0, RHIBufferUsage::kAccelerationStructure));
    mMemory->SetName("TLAS Memory");

    uint64 scratchSize = std::max(prebuildInfo.ScratchDataSizeInBytes, prebuildInfo.UpdateScratchDataSizeInBytes);
    mScratch = device->CreateBuffer(RHIBufferDesc(Align<uint>(scratchSize, 256), 0, RHIBufferUsage::kShaderWrite));
    mScratch->SetName("TLAS Scratch");

    mAlloc = device->GetBindlessManager()->WriteAS(this);
    mBindless.Index = mAlloc.Index;

    SERAPH_WHATEVER("Created D3D12 TLAS");
}

D3D12TLAS::~D3D12TLAS()
{
    mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mAlloc);
}
