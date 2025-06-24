//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:09:00
//

#include "D3D12BLAS.h"
#include "D3D12Device.h"

#undef max

D3D12BLAS::D3D12BLAS(D3D12Device* device, RHIBLASDesc desc)
{
    mDesc = desc;
    
    mGeometry = {};
    mGeometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    mGeometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
    mGeometry.Triangles.VertexBuffer.StartAddress = desc.VertexBuffer->GetAddress();
    mGeometry.Triangles.VertexBuffer.StrideInBytes = desc.VertexBuffer->GetDesc().Stride;
    mGeometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT; // Position format
    mGeometry.Triangles.VertexCount = desc.VertexCount;
    mGeometry.Triangles.IndexBuffer = desc.IndexBuffer->GetAddress();
    mGeometry.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
    mGeometry.Triangles.IndexCount = desc.IndexCount;
    mGeometry.Triangles.Transform3x4 = 0;

    mInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    mInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    mInputs.NumDescs = 1;
    mInputs.pGeometryDescs = &mGeometry;
    if (desc.Static) {
        mInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    } else {
        mInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    }

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
    device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&mInputs, &prebuildInfo);

    mMemory = device->CreateBuffer(RHIBufferDesc(Align<uint>(prebuildInfo.ResultDataMaxSizeInBytes, 256), 0, RHIBufferUsage::kAccelerationStructure));
    mMemory->SetName("BLAS Memory");

    uint64 scratchSize = std::max(prebuildInfo.ScratchDataSizeInBytes, prebuildInfo.UpdateScratchDataSizeInBytes);
    mScratch = device->CreateBuffer(RHIBufferDesc(Align<uint>(scratchSize, 256), 0, RHIBufferUsage::kShaderWrite));
    mScratch->SetName("BLAS Scratch");

    SERAPH_WHATEVER("Created D3D12 BLAS");
}

D3D12BLAS::~D3D12BLAS() 
{
    if (mMemory) delete mMemory;
    if (mScratch) delete mScratch;
}

uint64 D3D12BLAS::GetAddress()
{
    return mMemory->GetAddress();
}
