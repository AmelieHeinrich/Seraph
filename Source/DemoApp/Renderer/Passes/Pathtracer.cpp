//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:58:35
//

#include "Pathtracer.h"
#include "GBuffer.h"

#include <ImGui/imgui.h>

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
    RHIComputePipelineDesc desc = {};
    desc.PushConstantSize = sizeof(uint) * 12;

    PipelineReloader::SubscribeCompute("Pathtracer.hlsl", desc, "CSMain");
}

Pathtracer::~Pathtracer()
{
}

void Pathtracer::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Pathtracer");
    Pathtrace(begin);
    begin.CommandList->PopMarker();
}

void Pathtracer::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Pathtracer", ImGuiTreeNodeFlags_Framed)) {
        ImGui::SliderInt("Bounce Count", (int*)&mBounceCount, 1, 4);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
            ImGui::SetTooltip("How many times the ray should bounce before being terminated.");
        }

        ImGui::TreePop();
    }
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
            BindlessHandle Sampler;
            uint FrameCount;
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
            sampler.Sampler->GetBindlessHandle(),
            begin.FrameCount,
            mBounceCount
        };

        IRHIComputePipeline* pipeline = PipelineReloader::GetCompute("Pathtracer.hlsl");
        begin.CommandList->SetComputePipeline(pipeline);
        begin.CommandList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);

        // Insert manual UAV barrier
        RHIMemoryBarrier barrier(RHIResourceAccess::kShaderWrite, RHIResourceAccess::kShaderWrite, RHIPipelineStage::kComputeShader, RHIPipelineStage::kComputeShader);
        begin.CommandList->Barrier(barrier);
    }
    begin.CommandList->PopMarker();
}
