//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:17:38
//

#pragma once

#include "KGPU_CommandQueue.h"
#include "KGPU_Texture.h"
#include "KGPU_TextureView.h"
#include "KGPU_GraphicsPipeline.h"
#include "KGPU_Buffer.h"
#include "KGPU_ComputePipeline.h"
#include "KGPU_BLAS.h"
#include "KGPU_TLAS.h"

#undef MemoryBarrier

namespace KGPU
{
    enum class PipelineStage : uint64
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
    ENUM_CLASS_FLAGS(PipelineStage);
    
    enum class ResourceAccess : uint64
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
        kAccelerationStructureWrite  = BIT(17)
    };
    ENUM_CLASS_FLAGS(ResourceAccess);

    struct TextureBarrier
    {
        PipelineStage SourceStage;
        PipelineStage DestStage;
        ResourceAccess SourceAccess;
        ResourceAccess DestAccess;
        ResourceLayout SourceLayout = (ResourceLayout)123456789;
        ResourceLayout NewLayout;
        ITexture* Texture;
    
        uint BaseMipLevel = 0;
        uint LevelCount = 1;
        uint ArrayLayer = 0;
        uint LayerCount = 1;
    
        TextureBarrier() = default;
        TextureBarrier(ITexture* texture)
            : Texture(texture) {}
        TextureBarrier(ITexture* texture, ResourceAccess srcAccess, ResourceAccess dstAccess, PipelineStage srcStage, PipelineStage dstStage, ResourceLayout layout)
            : Texture(texture), SourceAccess(srcAccess), DestAccess(dstAccess), SourceStage(srcStage), DestStage(dstStage), NewLayout(layout) {}
    };
    
    struct BufferBarrier
    {
        PipelineStage SourceStage;
        PipelineStage DestStage;
        ResourceAccess SourceAccess;
        ResourceAccess DestAccess;
        IBuffer* Buffer;
    
        BufferBarrier() = default;
        BufferBarrier(IBuffer* buffer)
            : Buffer(buffer) {}
        BufferBarrier(IBuffer* buffer, ResourceAccess srcAccess, ResourceAccess dstAccess, PipelineStage srcStage, PipelineStage dstStage)
            : Buffer(buffer), SourceAccess(srcAccess), DestAccess(dstAccess), SourceStage(srcStage), DestStage(dstStage) {}
    };
    
    struct MemoryBarrier
    {
        ResourceAccess SourceAccess;
        ResourceAccess DestAccess;
        PipelineStage SourceStage;
        PipelineStage DestStage;
    
        MemoryBarrier() = default;
        MemoryBarrier(ResourceAccess srcAccess, ResourceAccess dstAccess, PipelineStage srcStage, PipelineStage dstStage)
            : SourceAccess(srcAccess), DestAccess(dstAccess), SourceStage(srcStage), DestStage(dstStage)
        {}
    };
    
    struct BarrierGroup
    {
        KC::Array<TextureBarrier> TextureBarriers;
        KC::Array<BufferBarrier> BufferBarriers;
        KC::Array<MemoryBarrier> MemoryBarriers;
    };
    
    struct RenderAttachment
    {
        ITextureView* View;
        bool Clear;
        float3 ClearValue;
    
        RenderAttachment() = default;
        RenderAttachment(ITextureView* view, bool clear = true, float3 clearValue = float3(0))
            : View(view), Clear(clear), ClearValue(clearValue) {}
    };
    
    struct RenderBegin
    {
        KC::Array<RenderAttachment> RenderTargets;
        RenderAttachment DepthTarget;
        uint Width;
        uint Height;
    
        RenderBegin() = default;
        RenderBegin(uint w, uint h, KC::Array<RenderAttachment> rts, RenderAttachment depth)
            : Width(w), Height(h), RenderTargets(rts), DepthTarget(depth) {}
    };
    
    enum class ASBuildMode
    {
        kRebuild,
        kRefit
    };
    
    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;
    
        virtual void Reset() = 0;
    
        virtual void Begin() = 0;
        virtual void End() = 0;
    
        virtual void BeginRendering(const RenderBegin& begin) = 0;
        virtual void EndRendering() = 0;
    
        virtual void Barrier(const TextureBarrier& barrier) = 0;
        virtual void Barrier(const BufferBarrier& barrier) = 0;
        virtual void Barrier(const MemoryBarrier& barrier) = 0;
        virtual void Barrier(const BarrierGroup& barrierGroup) = 0;
    
        virtual void ClearColor(ITextureView* view, float r, float g, float b) = 0;
    
        virtual void SetViewport(float width, float height, float topLeftX, float topLeftY) = 0;
        virtual void SetScissor(int left, int top, int right, int bottom) = 0;
        virtual void SetRenderSize(float width, float height) = 0;
        virtual void SetVertexBuffer(IBuffer* buffer, uint64 offset = 0) = 0;
        virtual void SetIndexBuffer(IBuffer* buffer) = 0;
        virtual void SetDepthBounds(float min, float max) {}

        virtual void SetGraphicsPipeline(IGraphicsPipeline* pipeline) = 0;
        virtual void SetGraphicsConstants(IGraphicsPipeline* pipeline, const void* data, uint64 size) = 0;
    
        virtual void SetComputePipeline(IComputePipeline* pipeline) = 0;
        virtual void SetComputeConstants(IComputePipeline* pipeline, const void* data, uint64 size) = 0;
    
        virtual void Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) = 0;
        virtual void DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance) = 0;
        virtual void Dispatch(uint x, uint y, uint z) = 0;
        virtual void DispatchMesh(uint x, uint y, uint z) = 0;
    
        virtual void CopyBufferToBufferFull(IBuffer* dest, IBuffer* src) = 0;
        virtual void CopyBufferToTexture(ITexture* dest, IBuffer* src, bool bufferHasMips = true) = 0;
        virtual void CopyTextureToBuffer(IBuffer* dest, ITexture* src) = 0;
        virtual void CopyTextureToTexture(ITexture* dst, ITexture* src) = 0;
        virtual void BuildBLAS(IBLAS* blas, ASBuildMode mode) = 0;
        virtual void BuildTLAS(ITLAS* blas, ASBuildMode mode, uint instanceCount, IBuffer* buffer) = 0;
    
        virtual void PushMarker(const KC::String& name) = 0;
        virtual void PopMarker() = 0;
    public:
        ICommandQueue* GetParentQueue() { return mParentQueue; }
    
    protected:
        ICommandQueue* mParentQueue;
    };

    struct ScopedMarker
    {
        ScopedMarker(ICommandList* list, const KC::String& name)
            : cmdList(list) {
            list->PushMarker(name);
        }

        ~ScopedMarker() {
            cmdList->PopMarker();
        }

        ICommandList* cmdList;
    };
}
