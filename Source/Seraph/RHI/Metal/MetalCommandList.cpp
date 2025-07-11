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
    : mSingleTime(singleTime), mParentDevice(device)
{
    SERAPH_WHATEVER("Created Metal command buffer!");
}

MetalCommandList::~MetalCommandList()
{

}

void MetalCommandList::Reset()
{

}

void MetalCommandList::Begin()
{

}

void MetalCommandList::End()
{

}

void MetalCommandList::BeginRendering(const RHIRenderBegin& begin)
{

}

void MetalCommandList::EndRendering()
{
    // Nothing
}

void MetalCommandList::Barrier(const RHITextureBarrier& barrier)
{

}


void MetalCommandList::Barrier(const RHIBufferBarrier& barrier)
{

}

void MetalCommandList::Barrier(const RHIMemoryBarrier& barrier)
{

}

void MetalCommandList::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{

}

void MetalCommandList::ClearColor(IRHITextureView* view, float r, float g, float b)
{

}

void MetalCommandList::SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline)
{

}

void MetalCommandList::SetViewport(float width, float height, float x, float y)
{

}

void MetalCommandList::SetVertexBuffer(IRHIBuffer* buffer)
{

}

void MetalCommandList::SetIndexBuffer(IRHIBuffer* buffer)
{

}

void MetalCommandList::SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size)
{

}

void MetalCommandList::SetComputePipeline(IRHIComputePipeline* pipeline)
{

}

void MetalCommandList::SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size)
{

}

void MetalCommandList::SetMeshPipeline(IRHIMeshPipeline* pipeline)
{

}

void MetalCommandList::SetMeshConstants(IRHIMeshPipeline* pipeline, const void *data, uint64 size)
{

}

void MetalCommandList::Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
{

}

void MetalCommandList::DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
{

}

void MetalCommandList::Dispatch(uint x, uint y, uint z)
{

}

void MetalCommandList::DispatchMesh(uint x, uint y, uint z)
{

}

void MetalCommandList::CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src)
{

}

void MetalCommandList::CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src)
{

}

void MetalCommandList::CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src)
{

}

void MetalCommandList::CopyTextureToTexture(IRHITexture* dst, IRHITexture* src)
{

}

void MetalCommandList::BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode)
{

}

void MetalCommandList::BuildTLAS(IRHITLAS* tlas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer)
{

}

void MetalCommandList::PushMarker(const std::string& name)
{

}

void MetalCommandList::PopMarker()
{

}

void MetalCommandList::BeginImGui()
{

}

void MetalCommandList::EndImGui()
{

}
