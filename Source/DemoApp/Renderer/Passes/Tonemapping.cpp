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

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint) * 4;
    mPipeline = mParentDevice->CreateComputePipeline(desc);
}

Tonemapping::~Tonemapping()
{
    delete mPipeline;
}

void Tonemapping::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Tonemapping");
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
