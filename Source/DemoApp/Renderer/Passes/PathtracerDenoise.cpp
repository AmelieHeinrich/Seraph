//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-24 19:26:16
//

#include "PathtracerDenoise.h"
#include "Pathtracer.h"

#include <imgui/imgui.h>

PathtracerDenoise::PathtracerDenoise(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Texture
    RHITextureDesc hdrDesc;
    hdrDesc.Width = width;
    hdrDesc.Height = height;
    hdrDesc.Format = RHITextureFormat::kR16G16B16A16_FLOAT;
    hdrDesc.Usage = RHITextureUsage::kShaderResource | RHITextureUsage::kStorage | RHITextureUsage::kRenderTarget;
    
    RendererResourceManager::CreateTexture(PATHTRACER_DENOISE_HISTORY_ID, hdrDesc);

    // Pipeline
    CompiledShader shader = ShaderCompiler::Compile("PathtracerDenoise", { "CSMain" });

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint) * 8;
    mPipeline = mParentDevice->CreateComputePipeline(desc);
}

PathtracerDenoise::~PathtracerDenoise()
{
    delete mPipeline;
}

void PathtracerDenoise::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Pathtracer Denoise");
    Denoise(begin);
    Copy(begin);
    begin.CommandList->PopMarker();
}

void PathtracerDenoise::UI(RenderPassBegin& begin)
{
}

void PathtracerDenoise::Denoise(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Accumulate");
    CODE_BLOCK("Execute") {
        if (begin.FrameCount > 0) {
            RendererResource& before = RendererResourceManager::Import(PATHTRACER_DENOISE_HISTORY_ID, begin.CommandList, RendererImportType::kShaderRead);
            RendererResource& now = RendererResourceManager::Import(PATHTRACER_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kShaderWrite);

            struct PushConstants {
                BindlessHandle Now;
                BindlessHandle Before;
                uint FrameCount;
                uint Pad;
            
                uint Width;
                uint Height;
                uint2 Pad2;
            } constants = {
                RendererViewRecycler::GetUAV(now.Texture)->GetBindlessHandle(),
                RendererViewRecycler::GetSRV(before.Texture)->GetBindlessHandle(),
                begin.FrameCount,
                0,

                mWidth,
                mHeight,
                {}
            };

            begin.CommandList->SetComputePipeline(mPipeline);
            begin.CommandList->SetComputeConstants(mPipeline, &constants, sizeof(constants));
            begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
        } else {
            RendererResource& before = RendererResourceManager::Import(PATHTRACER_DENOISE_HISTORY_ID, begin.CommandList, RendererImportType::kColorWrite);

            RHIRenderAttachment attachment(RendererViewRecycler::GetRTV(before.Texture));
            RHIRenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

            begin.CommandList->BeginRendering(renderBegin);
            begin.CommandList->EndRendering();
        }
    }
    begin.CommandList->PopMarker();
}

void PathtracerDenoise::Copy(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Copy History");
    CODE_BLOCK("Execute") {
        RendererResource& before = RendererResourceManager::Import(PATHTRACER_DENOISE_HISTORY_ID, begin.CommandList, RendererImportType::kTransferDest);
        RendererResource& now = RendererResourceManager::Import(PATHTRACER_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kTransferSource);

        begin.CommandList->CopyTextureToTexture(before.Texture, now.Texture);
    }
    begin.CommandList->PopMarker();
}
