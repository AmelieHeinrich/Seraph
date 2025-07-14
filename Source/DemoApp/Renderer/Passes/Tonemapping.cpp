//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:42:51
//

#include "Tonemapping.h"
#include "Deferred.h"
#include "GBuffer.h"
#include "Pathtracer.h"

#include <DemoApp/Renderer/Screenshotter.h>

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
    RendererResourceManager::CreateTexture(TONEMAPPING_SCREENSHOT_ID, hdrDesc);

    // Pipeline
    RHIComputePipelineDesc desc = {};
    desc.PushConstantSize = sizeof(uint) * 4;

    PipelineReloader::SubscribeCompute("Tonemapping.hlsl", desc, "CSMain");
}

Tonemapping::~Tonemapping()
{
}

void Tonemapping::Configure(RenderPath path)
{
    if (path == RenderPath::kBasic) mInputID = DEFERRED_HDR_TEXTURE_ID;
    else mInputID = PATHTRACER_HDR_TEXTURE_ID;
}

void Tonemapping::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Tonemapping");
    CODE_BLOCK("Tonemap") {
        RendererResource& hdr = RendererResourceManager::Import(mInputID, begin.CommandList, RendererImportType::kShaderRead);
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

        IRHIComputePipeline* pipeline = PipelineReloader::GetCompute("Tonemapping.hlsl");
        begin.CommandList->SetComputePipeline(pipeline);
        begin.CommandList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    CODE_BLOCK("Copy For Screenshots") {
        if (Screenshotter::WantsScreenshot()) {
            RendererResource& src = RendererResourceManager::Import(TONEMAPPING_LDR_ID, begin.CommandList, RendererImportType::kTransferSource);
            RendererResource& dst = RendererResourceManager::Import(TONEMAPPING_SCREENSHOT_ID, begin.CommandList, RendererImportType::kTransferDest);
            
            begin.CommandList->CopyTextureToTexture(dst.Texture, src.Texture);
        }
    }
    begin.CommandList->PopMarker();
}
