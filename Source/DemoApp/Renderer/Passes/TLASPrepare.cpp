//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:58:00
//

#include "TLASPrepare.h"

TLASPrepare::TLASPrepare(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
}

TLASPrepare::~TLASPrepare()
{
}

void TLASPrepare::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Build TLAS");
    CODE_BLOCK("Execute") {
        RHIBufferBarrier beforeBarrier(begin.RenderScene->GetTLAS()->GetMemory());
        beforeBarrier.SourceAccess = RHIResourceAccess::kAccelerationStructureRead;
        beforeBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureWrite;
        beforeBarrier.SourceStage = RHIPipelineStage::kComputeShader;
        beforeBarrier.DestStage = RHIPipelineStage::kAccelStructureWrite;

        RHIBufferBarrier afterBarrier(begin.RenderScene->GetTLAS()->GetMemory());
        afterBarrier.SourceAccess = RHIResourceAccess::kAccelerationStructureWrite;
        afterBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureRead;
        afterBarrier.SourceStage = RHIPipelineStage::kAccelStructureWrite;
        afterBarrier.DestStage = RHIPipelineStage::kComputeShader;

        begin.CommandList->Barrier(beforeBarrier);
        begin.CommandList->BuildTLAS(begin.RenderScene->GetTLAS(), RHIASBuildMode::kRebuild, begin.RenderScene->GetTLASInstances().size(), begin.RenderScene->GetInstanceBuffer());
        begin.CommandList->Barrier(afterBarrier);
    }
    begin.CommandList->PopMarker();
}
