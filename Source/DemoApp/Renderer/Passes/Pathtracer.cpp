//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:58:35
//

#include "Pathtracer.h"
#include "GBuffer.h"

Pathtracer::Pathtracer(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Texture
    RHITextureDesc hdrDesc;
    hdrDesc.Width = width;
    hdrDesc.Height = height;
    hdrDesc.Format = RHITextureFormat::kR16G16B16A16_FLOAT;
    hdrDesc.Usage = RHITextureUsage::kShaderResource | RHITextureUsage::kStorage;
    
    RendererResourceManager::CreateTexture(PATHTRACER_HDR_TEXTURE_ID, hdrDesc);

    // Pipeline
    CompiledShader shader = ShaderCompiler::Compile("Pathtracer", { "CSMain" });

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint) * 4;
    mPipeline = mParentDevice->CreateComputePipeline(desc);
}

Pathtracer::~Pathtracer()
{
    delete mPipeline;
}

void Pathtracer::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Pathtracer");
    CODE_BLOCK("Execute") {
        RendererResource& albedo = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& output = RendererResourceManager::Import(PATHTRACER_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kShaderWrite);

        struct PushConstants {
            uint Width;
            uint Height;
            BindlessHandle Output;
            BindlessHandle Albedo;
        } constants = {
            mWidth,
            mHeight,
            RendererViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(albedo.Texture)->GetBindlessHandle()
        };

        begin.CommandList->SetComputePipeline(mPipeline);
        begin.CommandList->SetComputeConstants(mPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}
