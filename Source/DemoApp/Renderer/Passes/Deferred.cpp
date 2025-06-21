//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:27:28
//

#include "Deferred.h"
#include "GBuffer.h"
#include "LightCulling.h"

#include <imgui/imgui.h>

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
    desc.PushConstantSize = sizeof(uint) * 20;
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
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        RendererResource& tileBuffer = RendererResourceManager::Import(LIGHT_CULL_CLUSTER_BUFFER, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& tileIndicesBuffer = RendererResourceManager::Import(LIGHT_CULL_CLUSTER_INDICES_BUFFER, begin.CommandList, RendererImportType::kShaderRead);

        RendererResource& depth = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& normal = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& albedo = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& pbr = RendererResourceManager::Import(GBUFFER_PBR_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& output = RendererResourceManager::Import(DEFERRED_HDR_TEXTURE_ID, begin.CommandList, RendererImportType::kShaderWrite);
        
        struct Constants {
            BindlessHandle depthHandle;
            BindlessHandle normalHandle;
            BindlessHandle albedoHandle;
            BindlessHandle pbrHandle;

            BindlessHandle outputHandle;
            uint width;
            uint height;
            BindlessHandle plArray;

            uint plCount;
            BindlessHandle camSRV;
            uint tileWidth;
            uint tileHeight;

            uint numTilesX;
            BindlessHandle binsArray;
            BindlessHandle tilesArray;
            uint pad;

            BindlessHandle slArray;
            uint slCount;
            uint clusterZ;
            uint clusterY;
        } constants = {
            RendererViewRecycler::GetTextureView(RHITextureViewDesc(depth.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(normal.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(albedo.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(pbr.Texture)->GetBindlessHandle(),

            RendererViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            mWidth,
            mHeight,
            begin.RenderScene->GetLights().GetPointLightBufferView(begin.FrameIndex)->GetBindlessHandle(),

            static_cast<uint>(begin.RenderScene->GetLights().PointLights.size()),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            CLUSTER_WIDTH,
            CLUSTER_HEIGHT,

            (mWidth + CLUSTER_WIDTH - 1) / CLUSTER_WIDTH,
            RendererViewRecycler::GetSRV(tileIndicesBuffer.Buffer)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(tileBuffer.Buffer)->GetBindlessHandle(),
            mShowTileHeatmap,

            begin.RenderScene->GetLights().GetSpotLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            static_cast<uint>(begin.RenderScene->GetLights().SpotLights.size()),
            CLUSTER_DEPTH,
            (mHeight + CLUSTER_HEIGHT - 1) / CLUSTER_HEIGHT
        };
    
        begin.CommandList->SetComputePipeline(mPipeline);
        begin.CommandList->SetComputeConstants(mPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}

void Deferred::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Deferred", ImGuiTreeNodeFlags_Framed)) {
        ImGui::Checkbox("Show Tile Heatmap", &mShowTileHeatmap);
        ImGui::TreePop();
    }
}
