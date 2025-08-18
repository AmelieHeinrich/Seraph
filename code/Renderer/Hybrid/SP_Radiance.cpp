//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:58:44
//

#include "SP_Radiance.h"
#include "SP_GBuffer.h"
#include "SP_Shadows.h"
#include "SP_LightCull.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <ToolDevConsole/TDC_Console.h>

#include <imgui.h>

namespace SP
{
    Radiance::Radiance()
    {
        TDC::Console::AddVariable("Graphics.Radiance.ShowTileHeatmap", mShowTileHeatmap);

        // Textures
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        // Texture
        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;

        Gfx::ResourceManager::CreateTexture(RADIANCE_HDR_TEXTURE_ID, hdrDesc);

        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/radiance/raster.kds");
    }

    Radiance::~Radiance()
    {
    }

    void Radiance::Render(RenderPassBegin& begin)
    {
        switch (mMode)
        {
            case RadianceMode::kRasterized: Rasterize(begin); break;
            case RadianceMode::kRaytraced: Raytrace(begin); break;
            case RadianceMode::kRaytracedReSTIR: RaytraceReSTIR(begin); break;
        }
    }

    void Radiance::Rasterize(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Radiance::Rasterize");

        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        Gfx::Resource& tileBuffer = Gfx::ResourceManager::Import(LIGHT_CULL_TILE_BUFFER, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& tileIndicesBuffer = Gfx::ResourceManager::Import(LIGHT_CULL_TILE_INDICES_BUFFER, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& depth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& normal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& albedo = Gfx::ResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& pbr = Gfx::ResourceManager::Import(GBUFFER_PBR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& output = Gfx::ResourceManager::Import(RADIANCE_HDR_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& shadowMask = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderRead);

        struct Constants {
            BindlessHandle depthHandle;
            BindlessHandle normalHandle;
            BindlessHandle albedoHandle;
            BindlessHandle pbrHandle;

            BindlessHandle outputHandle;
            int width;
            int height;
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
            BindlessHandle sunArray;
            BindlessHandle shadowMask;
        } constants = {
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(depth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(normal.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(albedo.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(pbr.Texture)->GetBindlessHandle(),

            Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            begin.Width,
            begin.Height,
            begin.World->GetLightList()->GetPointLightBufferView(begin.FrameIndex)->GetBindlessHandle(),

            static_cast<uint>(begin.World->GetLightList()->PointLights.size()),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            TILE_WIDTH,
            TILE_HEIGHT,

            (begin.Width + TILE_WIDTH - 1) / TILE_WIDTH,
            Gfx::ViewRecycler::GetSRV(tileIndicesBuffer.Buffer)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(tileBuffer.Buffer)->GetBindlessHandle(),
            mShowTileHeatmap,

            begin.World->GetLightList()->GetSpotLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            static_cast<uint>(begin.World->GetLightList()->SpotLights.size()),
            begin.World->GetLightList()->GetSunBufferView(begin.FrameIndex)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(shadowMask.Texture)->GetBindlessHandle(),
        };
    
        KGPU::IComputePipeline* pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/radiance/raster.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
    }

    void Radiance::Raytrace(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Radiance::Raytrace");
        Gfx::Resource& before = Gfx::ResourceManager::Import(RADIANCE_HDR_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void Radiance::RaytraceReSTIR(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Radiance::RaytraceReSTIR");
        Gfx::Resource& before = Gfx::ResourceManager::Import(RADIANCE_HDR_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void Radiance::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Radiance", ImGuiTreeNodeFlags_Framed)) {
            const char* modes[] = { "Rasterized", "Raytraced (UNIMPLEMENTED)", "Raytraced (ReSTIR) (UNIMPLEMENTED)" };
            if (ImGui::BeginCombo("Mode", modes[(int)mMode])) {
                for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
                    bool disabled = false;
                
                    // Disable RT modes if device doesn’t support raytracing
                    if ((i == (int)RadianceMode::kRaytraced || i == (int)RadianceMode::kRaytracedReSTIR) 
                        && !Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                        disabled = true;
                    }
                
                    if (disabled) {
                        ImGui::BeginDisabled();
                        ImGui::Selectable(modes[i], false);
                        ImGui::EndDisabled();
                    } else {
                        bool isSelected = (mMode == (RadianceMode)i);
                        if (ImGui::Selectable(modes[i], isSelected)) {
                            mMode = (RadianceMode)i;
                        }
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Show Tile Heatmap", &mShowTileHeatmap);
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                ImGui::SetTooltip("Used to visualize the repartition of point/spot lights throughout the screen tiles. Interpolates between blue, green, yellow and red depending on tile light  density.");
            }

            ImGui::TreePop();
        }
    }
}
