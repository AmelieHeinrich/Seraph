//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:12:03
//

//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:08:14
//

#pragma once

#include <RHI/CommandBuffer.h>

#include <Agility/d3d12.h>

class D3D12Device;
class D3D12CommandQueue;

class D3D12CommandBuffer : public IRHICommandBuffer
{
public:
    D3D12CommandBuffer(D3D12Device* device, D3D12CommandQueue* queue, bool singleTime);
    ~D3D12CommandBuffer();

    void Reset() override;
    void Begin() override;
    void End() override;

    void BeginRendering(const RHIRenderBegin& begin);
    void EndRendering();

    void Barrier(const RHITextureBarrier& barrier) override;
    void Barrier(const RHIBufferBarrier& barrier) override;
    void Barrier(const RHIMemoryBarrier& barrier) override;
    void BarrierGroup(const RHIBarrierGroup& barrierGroup) override;
    
    void ClearColor(IRHITextureView* view, float r, float g, float b) override;

    void SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline) override;
    void SetViewport(float width, float height, float x, float y) override;
    void SetVertexBuffer(IRHIBuffer* buffer) override;
    void SetIndexBuffer(IRHIBuffer* buffer) override;
    void SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size) override;
    void SetComputePipeline(IRHIComputePipeline* pipeline) override;
    void SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size) override;

    void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) override;
    void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance) override;
    void Dispatch(uint x, uint y, uint z) override;

    void CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src) override;
    void CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src) override;
    void CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src) override;
    void BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode) override;
    void BuildTLAS(IRHITLAS* blas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer) override;
    
    void PushMarker(const StringView& name) override;
    void PopMarker() override;

    void BeginImGui() override;
    void EndImGui() override;
public:
    ID3D12GraphicsCommandList10* GetList() { return mList; }

private:
    static D3D12_BARRIER_SYNC ToD3D12BarrierSync(RHIPipelineStage stage);
    static D3D12_BARRIER_ACCESS ToD3D12BarrierAccess(RHIResourceAccess access);
    static D3D12_BARRIER_LAYOUT ToD3D12BarrierLayout(RHIResourceLayout layout);

private:
    D3D12Device* mParentDevice;

    bool mSingleTime;

    ID3D12CommandAllocator* mAllocator = nullptr;
    ID3D12GraphicsCommandList10* mList = nullptr;
};
