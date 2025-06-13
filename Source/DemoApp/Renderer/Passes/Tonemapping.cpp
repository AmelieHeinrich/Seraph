//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:42:51
//

#include "Tonemapping.h"
#include "Deferred.h"
#include "GBuffer.h"

Tonemapping::Tonemapping(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Texture
    RHITextureDesc hdrDesc;
    hdrDesc.Width = width;
    hdrDesc.Height = height;
    hdrDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
    hdrDesc.Usage = RHITextureUsage::kShaderResource | RHITextureUsage::kStorage | RHITextureUsage::kRenderTarget;
    
    RendererResourceManager::CreateTexture(TONEMAPPING_LDR_ID, hdrDesc);

    // Pipeline
    CompiledShader shader = ShaderCompiler::Compile("Tonemapping", { "CSMain" });
    CompiledShader resolve = ShaderCompiler::Compile("RenderTexture", { "VSMain", "FSMain" });

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint) * 4;
    mPipeline = mParentDevice->CreateComputePipeline(desc);

    RHIGraphicsPipelineDesc resolveDesc = {};
    resolveDesc.Bytecode[ShaderStage::kVertex] = resolve.Entries["VSMain"];
    resolveDesc.Bytecode[ShaderStage::kFragment] = resolve.Entries["FSMain"];
    resolveDesc.PushConstantSize = sizeof(uint) * 2;
    resolveDesc.RenderTargetFormats.push_back(device->GetSurfaceFormat());
    mResolvePipeline = mParentDevice->CreateGraphicsPipeline(resolveDesc);
}

Tonemapping::~Tonemapping()
{
    delete mResolvePipeline;
    delete mPipeline;
}

void Tonemapping::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Tonemapping");
    Tonemap(begin);
    Copy(begin);
    begin.CommandList->PopMarker();
}

void Tonemapping::Tonemap(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Map to ACES");
    {
        RendererResource& hdr = RendererResourceManager::Import(DEFERRED_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& ldr = RendererResourceManager::Import(TONEMAPPING_LDR_ID, begin.CommandList, RendererImportType::kShaderWrite);

        struct Constants {
            BindlessHandle hdrHandle;
            BindlessHandle ldrHandle;
            uint width;
            uint height;
        } constants = {
            RendererViewRecycler::GetSRV(hdr.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(ldr.Texture)->GetBindlessHandle(),
            mWidth, mHeight
        };

        begin.CommandList->SetComputePipeline(mPipeline);
        begin.CommandList->SetComputeConstants(mPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}

void Tonemapping::Copy(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Resolve");
    {
        RHITextureBarrier swapchainBarrier(begin.SwapchainTexture);
        swapchainBarrier.SourceAccess = RHIResourceAccess::kNone;
        swapchainBarrier.DestAccess = RHIResourceAccess::kColorAttachmentWrite;
        swapchainBarrier.SourceStage = RHIPipelineStage::kNone;
        swapchainBarrier.DestStage = RHIPipelineStage::kColorAttachmentOutput;
        swapchainBarrier.NewLayout = RHIResourceLayout::kColorAttachment;

        RHIRenderBegin renderBegin(mWidth, mHeight, { RHIRenderAttachment(begin.SwapchainTextureView, false) }, {});

        RendererResource& ldr = RendererResourceManager::Import(TONEMAPPING_LDR_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& sampler = RendererResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);

        struct Constants {
            BindlessHandle in;
            BindlessHandle sampler;
        } constants = {
            RendererViewRecycler::GetSRV(ldr.Texture)->GetBindlessHandle(),
            sampler.Sampler->GetBindlessHandle()
        };

        begin.CommandList->Barrier(swapchainBarrier);
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->SetGraphicsPipeline(mResolvePipeline);
        begin.CommandList->SetViewport(mWidth, mHeight, 0, 0);
        begin.CommandList->SetGraphicsConstants(mResolvePipeline, &constants, sizeof(constants));
        begin.CommandList->Draw(3, 1, 0, 0);
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}
