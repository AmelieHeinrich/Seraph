//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:15:07
//

#include "MetalCommandList.h"
#include "MetalDevice.h"
#include "MetalCommandQueue.h"
#include "MetalTextureView.h"
#include "MetalTexture.h"
#include "MetalBuffer.h"
#include "MetalGraphicsPipeline.h"
#include "MetalComputePipeline.h"
#include "MetalMeshPipeline.h"
#include "MetalBLAS.h"
#include "MetalTLAS.h"

MetalCommandList::MetalCommandList(MetalDevice* device, MetalCommandQueue* queue, bool singleTime)
    : mSingleTime(singleTime), mParentDevice(device), mParentQueue(queue)
{
    SERAPH_WHATEVER("Created Metal command list!");
}

MetalCommandList::~MetalCommandList()
{

}

void MetalCommandList::Reset()
{
    // Do nothing
}

void MetalCommandList::Begin()
{
    mCommandBuffer = mParentQueue->GetQueue()->commandBuffer();
}

void MetalCommandList::End()
{
    // Do nothing
}

void MetalCommandList::BeginRendering(const RHIRenderBegin& begin)
{
    // TODO: Begin render encoder
}

void MetalCommandList::EndRendering()
{
    // TODO: End render encoder
}

void MetalCommandList::Barrier(const RHITextureBarrier& barrier)
{
    // Nothing
}

void MetalCommandList::Barrier(const RHIBufferBarrier& barrier)
{
    // Nothing
}

void MetalCommandList::Barrier(const RHIMemoryBarrier& barrier)
{
    // Nothing
}

void MetalCommandList::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{
    // Nothing
}

void MetalCommandList::ClearColor(IRHITextureView* view, float r, float g, float b)
{
    // Nothing
}

void MetalCommandList::SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline)
{
    // TODO
}

void MetalCommandList::SetViewport(float width, float height, float x, float y)
{
    // TODO
}

void MetalCommandList::SetVertexBuffer(IRHIBuffer* buffer)
{
    // TODO
}

void MetalCommandList::SetIndexBuffer(IRHIBuffer* buffer)
{
    // TODO
}

void MetalCommandList::SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size)
{
    // TODO
}

void MetalCommandList::SetComputePipeline(IRHIComputePipeline* pipeline)
{
    // TODO
}

void MetalCommandList::SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size)
{
    // TODO
}

void MetalCommandList::SetMeshPipeline(IRHIMeshPipeline* pipeline)
{
    // TODO
}

void MetalCommandList::SetMeshConstants(IRHIMeshPipeline* pipeline, const void *data, uint64 size)
{
    // TODO
}

void MetalCommandList::Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
{
    // TODO
}

void MetalCommandList::DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
{
    // TODO
}

void MetalCommandList::Dispatch(uint x, uint y, uint z)
{
    // TODO
}

void MetalCommandList::DispatchMesh(uint x, uint y, uint z)
{
    // TODO
}

void MetalCommandList::CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src)
{
    // TODO
}

void MetalCommandList::CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src)
{
    // TODO
}

void MetalCommandList::CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src)
{
    // TODO
}

void MetalCommandList::CopyTextureToTexture(IRHITexture* dst, IRHITexture* src)
{
    // TODO
}

void MetalCommandList::BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode)
{
    // TODO
}

void MetalCommandList::BuildTLAS(IRHITLAS* tlas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer)
{
    // TODO
}

void MetalCommandList::PushMarker(const std::string& name)
{
    // TODO
}

void MetalCommandList::PopMarker()
{
    // TODO
}

void MetalCommandList::BeginImGui()
{
    // TODO
}

void MetalCommandList::EndImGui()
{
    // TODO
}
