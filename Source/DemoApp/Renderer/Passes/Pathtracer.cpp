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
    desc.PushConstantSize = sizeof(uint) * 12;
    mPipeline = mParentDevice->CreateComputePipeline(desc);
}

Pathtracer::~Pathtracer()
{
    delete mPipeline;
}

void Pathtracer::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Pathtracer");
    BuildTLAS(begin);
    Pathtrace(begin);
    begin.CommandList->PopMarker();
}

void Pathtracer::BuildTLAS(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Build TLAS");
    CODE_BLOCK("Execute") {
        RHIBufferBarrier beforeBarrier(begin.RenderScene->GetTLAS()->GetMemory());
        beforeBarrier.SourceAccess = RHIResourceAccess::kAccelerationStructureRead;
        beforeBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureWrite;
        beforeBarrier.SourceStage = RHIPipelineStage::kComputeShader;
        beforeBarrier.DestStage = RHIPipelineStage::kAccelStructureWrite;

        RHIBufferBarrier afterBarrier(begin.RenderScene->GetTLAS()->GetMemory());
        afterBarrier.SourceAccess = RHIResourceAccess::kAccelerationStructureWrite;
        afterBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureRead;
        afterBarrier.SourceStage = RHIPipelineStage::kAccelStructureWrite;
        afterBarrier.DestStage = RHIPipelineStage::kComputeShader;

        begin.CommandList->Barrier(beforeBarrier);
        begin.CommandList->BuildTLAS(begin.RenderScene->GetTLAS(), RHIASBuildMode::kRebuild, begin.RenderScene->GetTLASInstances().size(), begin.RenderScene->GetInstanceBuffer());
        begin.CommandList->Barrier(afterBarrier);
    }
    begin.CommandList->PopMarker();
}

void Pathtracer::Pathtrace(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Trace Rays");
    CODE_BLOCK("Execute") {
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        RendererResource& depth = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& albedo = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& normal = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& output = RendererResourceManager::Import(PATHTRACER_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& sampler = RendererResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);

        struct PushConstants {
            uint Width;
            uint Height;
            BindlessHandle Output;
            BindlessHandle Albedo;

            BindlessHandle Camera;
            BindlessHandle Depth;
            BindlessHandle AccelerationStructure;
            BindlessHandle Normal;

            BindlessHandle SceneInstances;
            BindlessHandle SceneMaterials;
            BindlessHandle Sampler;
            uint Pad;
        } constants = {
            mWidth,
            mHeight,
            RendererViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(albedo.Texture)->GetBindlessHandle(),

            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            RendererViewRecycler::GetTextureView(RHITextureViewDesc(depth.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            begin.RenderScene->GetTLAS()->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(normal.Texture)->GetBindlessHandle(),

            RendererViewRecycler::GetSRV(begin.RenderScene->GetSceneInstanceBuffer())->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(begin.RenderScene->GetSceneMaterialBuffer())->GetBindlessHandle(),
            sampler.Sampler->GetBindlessHandle(),
            0
        };

        begin.CommandList->SetComputePipeline(mPipeline);
        begin.CommandList->SetComputeConstants(mPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}
