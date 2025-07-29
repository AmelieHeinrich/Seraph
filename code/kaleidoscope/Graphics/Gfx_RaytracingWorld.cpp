//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:36:44
//

#include "Gfx_RaytracingWorld.h"

namespace Gfx
{
    RaytracingWorld::RaytracingWorld()
    {
        KGPU::IDevice* device = Manager::GetDevice();

        mTLAS = device->CreateTLAS();
        mInstanceBuffer = device->CreateBuffer(KGPU::BufferDesc(sizeof(KGPU::TLASInstance) * KGPU::MAX_TLAS_INSTANCES, sizeof(KGPU::TLASInstance), KGPU::BufferUsage::kConstant));
    }

    RaytracingWorld::~RaytracingWorld()
    {
        KC_DELETE(mInstanceBuffer);
        KC_DELETE(mTLAS);
    }

    void RaytracingWorld::Reset()
    {
        mInstances.clear();
    }

    void RaytracingWorld::AddInstance(Gfx::MeshPrimitive* primitive, KGPU::float4x4 transform, bool opaque)
    {
        if (!Gfx::Manager::GetDevice()->SupportsRaytracing())
            return;

        KGPU::TLASInstance instance;
        instance.AccelerationStructureReference = primitive->GetBLAS()->GetAddress();
        instance.Mask = 1;
        instance.Transform = KGPU::float3x4(transform);
        instance.Flags = opaque ? KGPU::TLAS_INSTANCE_OPAQUE : KGPU::TLAS_INSTANCE_NON_OPAQUE;
        
        mInstances.push_back(instance);
    }

    void RaytracingWorld::Build(KGPU::ICommandList* list)
    {
        void* ptr = mInstanceBuffer->Map();
        memcpy(ptr, mInstances.data(), mInstances.size() * sizeof(KGPU::TLASInstance));
        mInstanceBuffer->Unmap();

        KGPU::BufferBarrier beforeBarrier(mTLAS->GetMemory());
        beforeBarrier.SourceAccess = KGPU::ResourceAccess::kAccelerationStructureRead;
        beforeBarrier.DestAccess = KGPU::ResourceAccess::kAccelerationStructureWrite;
        beforeBarrier.SourceStage = KGPU::PipelineStage::kComputeShader;
        beforeBarrier.DestStage = KGPU::PipelineStage::kAccelStructureWrite;

        KGPU::BufferBarrier afterBarrier(mTLAS->GetMemory());
        afterBarrier.SourceAccess = KGPU::ResourceAccess::kAccelerationStructureWrite;
        afterBarrier.DestAccess = KGPU::ResourceAccess::kAccelerationStructureRead;
        afterBarrier.SourceStage = KGPU::PipelineStage::kAccelStructureWrite;
        afterBarrier.DestStage = KGPU::PipelineStage::kComputeShader;

        list->PushMarker("Gfx::RaytracingWorld::Build");
        list->Barrier(beforeBarrier);
        list->BuildTLAS(mTLAS, KGPU::ASBuildMode::kRebuild, mInstances.size(), mInstanceBuffer);
        list->Barrier(afterBarrier);
        list->PopMarker();
    }
}
