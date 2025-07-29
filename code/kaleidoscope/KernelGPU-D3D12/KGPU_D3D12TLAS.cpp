//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:06:13
//

#include "KGPU_D3D12TLAS.h"
#include "KGPU_D3D12Device.h"

#undef max

namespace KGPU
{
    D3D12TLAS::D3D12TLAS(D3D12Device* device)
        : mParentDevice(device)
    {
        if (!device->SupportsRaytracing())
            return;

        mInputs = {};
        mInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        mInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        mInputs.NumDescs = MAX_TLAS_INSTANCES;
    
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
        device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&mInputs, &prebuildInfo);
    
        mMemory = device->CreateBuffer(BufferDesc(KOS::Align<uint>(prebuildInfo.ResultDataMaxSizeInBytes, 256), 0, BufferUsage::kAccelerationStructure));
        mMemory->SetName("TLAS Memory");
    
        uint64 scratchSize = std::max(prebuildInfo.ScratchDataSizeInBytes, prebuildInfo.UpdateScratchDataSizeInBytes);
        mScratch = device->CreateBuffer(BufferDesc(KOS::Align<uint>(scratchSize, 256), 0, BufferUsage::kShaderWrite));
        mScratch->SetName("TLAS Scratch");
    
        mAlloc = device->GetBindlessManager()->WriteAS(this);
        mBindless.Index = mAlloc.Index;
    
        KD_WHATEVER("Created D3D12 TLAS");
    }
    
    D3D12TLAS::~D3D12TLAS()
    {
        if (!mParentDevice->SupportsRaytracing())
            return;

        mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mAlloc);

        KC_DELETE(mMemory);
        KC_DELETE(mScratch);
    }
}
