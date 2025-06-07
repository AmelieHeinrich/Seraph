//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:27:28
//

#include "Deferred.h"
#include "GBuffer.h"

Deferred::Deferred(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Texture
    RHITextureDesc hdrDesc;
    hdrDesc.Width = width;
    hdrDesc.Height = height;
    hdrDesc.Format = RHITextureFormat::kR16G16B16A16_FLOAT;
    hdrDesc.Usage = RHITextureUsage::kShaderResource | RHITextureUsage::kStorage;
    
    RendererResourceManager::CreateTexture(DEFERRED_HDR_TEXTURE_ID, hdrDesc);

    // Pipeline
    CompiledShader shader = ShaderCompiler::Compile("Deferred", { "CSMain" });

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint) * 8;
    mPipeline = mParentDevice->CreateComputePipeline(desc);
}

Deferred::~Deferred()
{
    delete mPipeline;
}

void Deferred::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Deferred");
    {
        RendererResource& depth = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& normal = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& albedo = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& output = RendererResourceManager::Import(DEFERRED_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kShaderWrite);
        
        struct Constants {
            BindlessHandle depthHandle;
            BindlessHandle normalHandle;
            BindlessHandle albedoHandle;
            BindlessHandle outputHandle;
        
            uint width;
            uint height;
            glm::uvec2 pad;
        } constants = {
            RendererViewRecycler::GetTextureView(RHITextureViewDesc(depth.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(normal.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(albedo.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
        
            mWidth, mHeight
        };
    
        begin.CommandList->SetComputePipeline(mPipeline);
        begin.CommandList->SetComputeConstants(mPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}
