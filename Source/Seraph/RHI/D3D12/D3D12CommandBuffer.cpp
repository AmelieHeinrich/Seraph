//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:15:07
//

#include "D3D12CommandBuffer.h"

D3D12CommandBuffer::D3D12CommandBuffer(D3D12Device* device, D3D12CommandQueue* queue, bool singleTime)
{

}

D3D12CommandBuffer::~D3D12CommandBuffer()
{

}

void D3D12CommandBuffer::Reset()
{

}

void D3D12CommandBuffer::Begin()
{

}

void D3D12CommandBuffer::End()
{

}

void D3D12CommandBuffer::BeginRendering(const RHIRenderBegin& begin)
{

}

void D3D12CommandBuffer::EndRendering()
{

}

void D3D12CommandBuffer::Barrier(const RHITextureBarrier& barrier)
{

}

void D3D12CommandBuffer::Barrier(const RHIBufferBarrier& barrier)
{

}

void D3D12CommandBuffer::Barrier(const RHIMemoryBarrier& barrier)
{

}

void D3D12CommandBuffer::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{

}

void D3D12CommandBuffer::ClearColor(IRHITextureView* view, float r, float g, float b)
{

}

void D3D12CommandBuffer::SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline)
{

}

void D3D12CommandBuffer::SetViewport(float width, float height, float x, float y)
{

}

void D3D12CommandBuffer::SetVertexBuffer(IRHIBuffer* buffer)
{

}

void D3D12CommandBuffer::SetIndexBuffer(IRHIBuffer* buffer)
{

}

void D3D12CommandBuffer::SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size)
{

}

void D3D12CommandBuffer::SetComputePipeline(IRHIComputePipeline* pipeline)
{

}

void D3D12CommandBuffer::SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size)
{

}

void D3D12CommandBuffer::Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
{

}

void D3D12CommandBuffer::DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
{

}

void D3D12CommandBuffer::Dispatch(uint x, uint y, uint z)
{

}

void D3D12CommandBuffer::CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src)
{

}

void D3D12CommandBuffer::CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src)
{

}

void D3D12CommandBuffer::BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode)
{

}

void D3D12CommandBuffer::BuildTLAS(IRHITLAS* blas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer)
{

}

void D3D12CommandBuffer::PushMarker(const StringView& name)
{

}

void D3D12CommandBuffer::PopMarker()
{

}
