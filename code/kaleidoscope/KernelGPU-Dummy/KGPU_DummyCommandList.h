//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:14:20
//

#pragma once

#include <KernelGPU/KGPU_CommandList.h>

namespace KGPU
{
    class DummyDevice;
    class DummyCommandQueue;

    class DummyCommandList : public ICommandList
    {
    public:
        DummyCommandList(DummyDevice* device, DummyCommandQueue* queue, bool singleTime);
        ~DummyCommandList();

        void Reset() override;
        void Begin() override;
        void End() override;

        void BeginRendering(const RenderBegin& begin);
        void EndRendering();

        void Barrier(const TextureBarrier& barrier) override;
        void Barrier(const BufferBarrier& barrier) override;
        void Barrier(const MemoryBarrier& barrier) override;
        void Barrier(const BarrierGroup& barrierGroup) override;

        void ClearColor(ITextureView* view, float r, float g, float b) override;

        void SetGraphicsPipeline(IGraphicsPipeline* pipeline) override;
        void SetViewport(float width, float height, float topLeftX, float topLeftY) override;
        void SetScissor(int left, int top, int right, int bottom) override;
        void SetRenderSize(float width, float height) override;
        void SetVertexBuffer(IBuffer* buffer, uint64 offset = 0) override;
        void SetIndexBuffer(IBuffer* buffer) override;
        void SetGraphicsConstants(IGraphicsPipeline* pipeline, const void* data, uint64 size) override;
        void SetComputePipeline(IComputePipeline* pipeline) override;
        void SetComputeConstants(IComputePipeline* pipeline, const void* data, uint64 size) override;

        void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) override;
        void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance) override;
        void Dispatch(uint x, uint y, uint z) override;
        void DispatchMesh(uint x, uint y, uint z) override;

        void CopyBufferToBufferFull(IBuffer* dest, IBuffer* src) override;
        void CopyBufferToTexture(ITexture* dest, IBuffer* src, bool bufferHasMips = true) override;
        void CopyTextureToBuffer(IBuffer* dest, ITexture* src) override;
        void CopyTextureToTexture(ITexture* dst, ITexture* src) override;
        void BuildBLAS(IBLAS* blas, ASBuildMode mode) override;
        void BuildTLAS(ITLAS* blas, ASBuildMode mode, uint instanceCount, IBuffer* buffer) override;

        void PushMarker(const KC::String& name) override;
        void PopMarker() override;

    private:
        DummyDevice* mParentDevice;
    };
}
