//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 22:07:33
//

#include "CopyToSwapchain.h"
#include "GBuffer.h"
#include "Tonemapping.h"

CopyToSwapchain::CopyToSwapchain(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    RHIGraphicsPipelineDesc resolveDesc = {};
    resolveDesc.PushConstantSize = sizeof(uint) * 2;
    resolveDesc.RenderTargetFormats.push_back(device->GetSurfaceFormat());

    PipelineReloader::SubscribeGraphics("RenderTexture.hlsl", resolveDesc, { "VSMain", "FSMain" });
}

CopyToSwapchain::~CopyToSwapchain()
{
}

void CopyToSwapchain::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Copy To Swapchain");
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
        IRHIGraphicsPipeline* pipeline = PipelineReloader::GetGraphics("RenderTexture.hlsl");

        struct Constants {
            BindlessHandle in;
            BindlessHandle sampler;
        } constants = {
            RendererViewRecycler::GetSRV(ldr.Texture)->GetBindlessHandle(),
            sampler.Sampler->GetBindlessHandle()
        };

        begin.CommandList->Barrier(swapchainBarrier);
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->SetGraphicsPipeline(pipeline);
        begin.CommandList->SetViewport(mWidth, mHeight, 0, 0);
        begin.CommandList->SetGraphicsConstants(pipeline, &constants, sizeof(constants));
        begin.CommandList->Draw(3, 1, 0, 0);
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}
