//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:15:07
//

#include "DummyCommandList.h"
#include "DummyDevice.h"
#include "DummyCommandQueue.h"
#include "DummyTextureView.h"
#include "DummyTexture.h"
#include "DummyBuffer.h"
#include "DummyGraphicsPipeline.h"
#include "DummyComputePipeline.h"
#include "DummyMeshPipeline.h"
#include "DummyBLAS.h"
#include "DummyTLAS.h"

DummyCommandList::DummyCommandList(DummyDevice* device, DummyCommandQueue* queue, bool singleTime)
    : mSingleTime(singleTime), mParentDevice(device)
{
    SERAPH_WHATEVER("Created Dummy command buffer!");
}

DummyCommandList::~DummyCommandList()
{

}

void DummyCommandList::Reset()
{

}

void DummyCommandList::Begin()
{

}

void DummyCommandList::End()
{

}

void DummyCommandList::BeginRendering(const RHIRenderBegin& begin)
{

}

void DummyCommandList::EndRendering()
{
    // Nothing
}

void DummyCommandList::Barrier(const RHITextureBarrier& barrier)
{

}


void DummyCommandList::Barrier(const RHIBufferBarrier& barrier)
{

}

void DummyCommandList::Barrier(const RHIMemoryBarrier& barrier)
{

}

void DummyCommandList::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{

}

void DummyCommandList::ClearColor(IRHITextureView* view, float r, float g, float b)
{

}

void DummyCommandList::SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline)
{

}

void DummyCommandList::SetViewport(float width, float height, float x, float y)
{

}

void DummyCommandList::SetVertexBuffer(IRHIBuffer* buffer)
{

}

void DummyCommandList::SetIndexBuffer(IRHIBuffer* buffer)
{

}

void DummyCommandList::SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size)
{

}

void DummyCommandList::SetComputePipeline(IRHIComputePipeline* pipeline)
{

}

void DummyCommandList::SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size)
{

}

void DummyCommandList::SetMeshPipeline(IRHIMeshPipeline* pipeline)
{

}

void DummyCommandList::SetMeshConstants(IRHIMeshPipeline* pipeline, const void *data, uint64 size)
{

}

void DummyCommandList::Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
{

}

void DummyCommandList::DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
{

}

void DummyCommandList::Dispatch(uint x, uint y, uint z)
{

}

void DummyCommandList::DispatchMesh(uint x, uint y, uint z)
{

}

void DummyCommandList::CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src)
{

}

void DummyCommandList::CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src)
{

}

void DummyCommandList::CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src)
{

}

void DummyCommandList::CopyTextureToTexture(IRHITexture* dst, IRHITexture* src)
{

}

void DummyCommandList::BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode)
{

}

void DummyCommandList::BuildTLAS(IRHITLAS* tlas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer)
{

}

void DummyCommandList::PushMarker(const std::string& name)
{

}

void DummyCommandList::PopMarker()
{

}

void DummyCommandList::BeginImGui()
{

}

void DummyCommandList::EndImGui()
{

}
