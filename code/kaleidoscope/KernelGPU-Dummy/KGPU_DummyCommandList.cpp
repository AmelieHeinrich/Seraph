//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:17:26
//

#include "KGPU_DummyCommandList.h"
#include "KGPU_DummyDevice.h"
#include "KGPU_DummyCommandQueue.h"
#include "KGPU_DummyTextureView.h"
#include "KGPU_DummyTexture.h"
#include "KGPU_DummyBuffer.h"
#include "KGPU_DummyGraphicsPipeline.h"
#include "KGPU_DummyComputePipeline.h"
#include "KGPU_DummyBLAS.h"
#include "KGPU_DummyTLAS.h"

namespace KGPU
{
    DummyCommandList::DummyCommandList(DummyDevice* device, DummyCommandQueue* queue, bool singleTime)
        : mParentDevice(device)
    {
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

    void DummyCommandList::BeginRendering(const RenderBegin& begin)
    {
    }

    void DummyCommandList::EndRendering()
    {
    }

    void DummyCommandList::Barrier(const TextureBarrier& barrier)
    {
    }


    void DummyCommandList::Barrier(const BufferBarrier& barrier)
    {
    }

    void DummyCommandList::Barrier(const MemoryBarrier& barrier)
    {
    }

    void DummyCommandList::Barrier(const BarrierGroup& barrierGroup)
    {
    }

    void DummyCommandList::ClearColor(ITextureView* view, float r, float g, float b)
    {
    }

    void DummyCommandList::SetGraphicsPipeline(IGraphicsPipeline* pipeline)
    {
    }

    void DummyCommandList::SetViewport(float width, float height, float x, float y)
    {
    }

    void DummyCommandList::SetScissor(int left, int top, int right, int bottom)
    {
    }

    void DummyCommandList::SetRenderSize(float width, float height)
    {
    }

    void DummyCommandList::SetVertexBuffer(IBuffer* buffer, uint64 offset)
    {
    }

    void DummyCommandList::SetIndexBuffer(IBuffer* buffer)
    {
    }

    void DummyCommandList::SetGraphicsConstants(IGraphicsPipeline* pipeline, const void* data, uint64 size)
    {
    }

    void DummyCommandList::SetComputePipeline(IComputePipeline* pipeline)
    {
    }

    void DummyCommandList::SetComputeConstants(IComputePipeline* pipeline, const void* data, uint64 size)
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

    void DummyCommandList::CopyBufferToBufferFull(IBuffer* dest, IBuffer* src)
    {
    }

    void DummyCommandList::CopyBufferToTexture(ITexture* dest, IBuffer* src, bool bufferHasMips)
    {
    }

    void DummyCommandList::CopyTextureToBuffer(IBuffer* dest, ITexture* src)
    {
    }

    void DummyCommandList::CopyTextureToTexture(ITexture* dst, ITexture* src)
    {
    }

    void DummyCommandList::BuildBLAS(IBLAS* blas, ASBuildMode mode)
    {
    }

    void DummyCommandList::BuildTLAS(ITLAS* tlas, ASBuildMode mode, uint instanceCount, IBuffer* buffer)
    {
    }

    void DummyCommandList::PushMarker(const KC::String& name)
    {
    }

    void DummyCommandList::PopMarker()
    {
    }
}
