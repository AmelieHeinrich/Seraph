//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 18:06:29
//

#pragma once

#include "CommandQueue.h"
#include "Texture.h"
#include "TextureView.h"
#include "GraphicsPipeline.h"
#include "Buffer.h"
#include "ComputePipeline.h"
#include "BLAS.h"
#include "TLAS.h"
#include "MeshPipeline.h"

enum class RHIPipelineStage : uint64
{
    kNone                    = 0,
    kTopOfPipe               = BIT(0),
    kDrawIndirect            = BIT(1),
    kVertexInput             = BIT(2),
    kVertexShader            = BIT(3),
    kHullShader              = BIT(4),
    kDomainShader            = BIT(5),
    kGeometryShader          = BIT(6),
    kPixelShader             = BIT(7),
    kComputeShader           = BIT(8),
    kRayTracingShader        = BIT(9),
    kEarlyFragmentTests      = BIT(10),
    kLateFragmentTests       = BIT(11),
    kColorAttachmentOutput   = BIT(12),
    kResolve                 = BIT(13),
    kBottomOfPipe            = BIT(14),
    kCopy                    = BIT(15),
    kAllGraphics             = BIT(16),
    kAllCommands             = BIT(17),
    kAccelStructureWrite     = BIT(18),
    kIndexInput              = BIT(19)
};
ENUM_CLASS_FLAGS(RHIPipelineStage);

enum class RHIResourceAccess : uint64
{
    kNone                        = 0,
    kIndirectCommandRead         = BIT(0),
    kVertexBufferRead            = BIT(1),
    kIndexBufferRead             = BIT(2),
    kConstantBufferRead          = BIT(3),
    kShaderRead                  = BIT(4),
    kShaderWrite                 = BIT(5),
    kColorAttachmentRead         = BIT(6),
    kColorAttachmentWrite        = BIT(7),
    kDepthStencilRead            = BIT(8),
    kDepthStencilWrite           = BIT(9),
    kTransferRead                = BIT(10),
    kTransferWrite               = BIT(11),
    kHostRead                    = BIT(12),
    kHostWrite                   = BIT(13),
    kMemoryRead                  = BIT(14),
    kMemoryWrite                 = BIT(15),
    kAccelerationStructureRead   = BIT(16),
    kAccelerationStructureWrite  = BIT(17),
};
ENUM_CLASS_FLAGS(RHIResourceAccess);

struct RHITextureBarrier
{
    RHIPipelineStage SourceStage;
    RHIPipelineStage DestStage;
    RHIResourceAccess SourceAccess;
    RHIResourceAccess DestAccess;
    RHIResourceLayout NewLayout;
    IRHITexture* Texture;

    uint BaseMipLevel = 0;
    uint LevelCount = 1;
    uint ArrayLayer = 0;
    uint LayerCount = 1;

    RHITextureBarrier() = default;
    RHITextureBarrier(IRHITexture* texture)
        : Texture(texture) {}
    RHITextureBarrier(IRHITexture* texture, RHIResourceAccess srcAccess, RHIResourceAccess dstAccess, RHIPipelineStage srcStage, RHIPipelineStage dstStage, RHIResourceLayout layout)
        : Texture(texture), SourceAccess(srcAccess), DestAccess(dstAccess), SourceStage(srcStage), DestStage(dstStage), NewLayout(layout) {}
};

struct RHIBufferBarrier
{
    RHIPipelineStage SourceStage;
    RHIPipelineStage DestStage;
    RHIResourceAccess SourceAccess;
    RHIResourceAccess DestAccess;
    IRHIBuffer* Buffer;

    RHIBufferBarrier() = default;
    RHIBufferBarrier(IRHIBuffer* buffer)
        : Buffer(buffer) {}
    RHIBufferBarrier(IRHIBuffer* buffer, RHIResourceAccess srcAccess, RHIResourceAccess dstAccess, RHIPipelineStage srcStage, RHIPipelineStage dstStage)
        : Buffer(buffer), SourceAccess(srcAccess), DestAccess(dstAccess), SourceStage(srcStage), DestStage(dstStage) {}
};

struct RHIMemoryBarrier
{
    RHIResourceAccess SourceAccess;
    RHIResourceAccess DestAccess;
    RHIPipelineStage SourceStage;
    RHIPipelineStage DestStage;

    RHIMemoryBarrier() = default;
    RHIMemoryBarrier(RHIResourceAccess srcAccess, RHIResourceAccess dstAccess, RHIPipelineStage srcStage, RHIPipelineStage dstStage)
        : SourceAccess(srcAccess), DestAccess(dstAccess), SourceStage(srcStage), DestStage(dstStage)
    {}
};

struct RHIBarrierGroup
{
    Array<RHITextureBarrier> TextureBarriers;
    Array<RHIBufferBarrier> BufferBarriers;
    Array<RHIMemoryBarrier> MemoryBarriers;
};

struct RHIRenderAttachment
{
    IRHITextureView* View;
    bool Clear;

    RHIRenderAttachment() = default;
    RHIRenderAttachment(IRHITextureView* view, bool clear = true)
        : View(view), Clear(clear) {}
};

struct RHIRenderBegin
{
    Array<RHIRenderAttachment> RenderTargets;
    RHIRenderAttachment DepthTarget;
    uint Width;
    uint Height;

    RHIRenderBegin() = default;
    RHIRenderBegin(uint w, uint h, Array<RHIRenderAttachment> rts, RHIRenderAttachment depth)
        : Width(w), Height(h), RenderTargets(rts), DepthTarget(depth) {}
};

enum class RHIASBuildMode
{
    kRebuild,
    kRefit
};

class IRHICommandList
{
public:
    ~IRHICommandList() = default;

    virtual void Reset() = 0;

    virtual void Begin() = 0;
    virtual void End() = 0;

    virtual void BeginRendering(const RHIRenderBegin& begin) = 0;
    virtual void EndRendering() = 0;

    virtual void Barrier(const RHITextureBarrier& barrier) = 0;
    virtual void Barrier(const RHIBufferBarrier& barrier) = 0;
    virtual void Barrier(const RHIMemoryBarrier& barrier) = 0;
    virtual void BarrierGroup(const RHIBarrierGroup& barrierGroup) = 0;

    virtual void ClearColor(IRHITextureView* view, float r, float g, float b) = 0;

    virtual void SetViewport(float width, float height, float x, float y) = 0;
    virtual void SetVertexBuffer(IRHIBuffer* buffer) = 0;
    virtual void SetIndexBuffer(IRHIBuffer* buffer) = 0;

    virtual void SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline) = 0;
    virtual void SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size) = 0;

    virtual void SetComputePipeline(IRHIComputePipeline* pipeline) = 0;
    virtual void SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size) = 0;

    virtual void SetMeshPipeline(IRHIMeshPipeline* pipeline) = 0;
    virtual void SetMeshConstants(IRHIMeshPipeline* pipeline, const void *data, uint64 size) = 0;

    virtual void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) = 0;
    virtual void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance) = 0;
    virtual void Dispatch(uint x, uint y, uint z) = 0;
    virtual void DispatchMesh(uint x, uint y, uint z) = 0;

    virtual void CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src) = 0;
    virtual void CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src) = 0;
    virtual void CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src) = 0;
    virtual void CopyTextureToTexture(IRHITexture* dst, IRHITexture* src) = 0;
    virtual void BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode) = 0;
    virtual void BuildTLAS(IRHITLAS* blas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer) = 0;

    virtual void PushMarker(const String& name) = 0;
    virtual void PopMarker() = 0;
    
    virtual void BeginImGui() = 0;
    virtual void EndImGui() = 0;
public:
    IRHICommandQueue* GetParentQueue() { return mParentQueue; }

protected:
    IRHICommandQueue* mParentQueue;
};
