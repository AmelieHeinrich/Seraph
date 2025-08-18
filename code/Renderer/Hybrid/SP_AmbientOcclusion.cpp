//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#include "SP_AmbientOcclusion.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ViewRecycler.h>
#include <imgui.h>

namespace SP
{
    AmbientOcclusion::AmbientOcclusion()
    {
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        // Texture
        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.Format = KGPU::TextureFormat::kR32_FLOAT;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;

        Gfx::ResourceManager::CreateTexture(AMBIENT_OCCLUSION_MASK_ID, hdrDesc);
    }

    AmbientOcclusion::~AmbientOcclusion()
    {

    }

    void AmbientOcclusion::Render(RenderPassBegin& begin)
    {
        switch (mMode)
        {
            case AmbientOcclusionMode::kNone: None(begin); break;
            case AmbientOcclusionMode::kSSAO: SSAO(begin); break;
            case AmbientOcclusionMode::kGTAO: GTAO(begin); break;
            case AmbientOcclusionMode::kRTAO: RTAO(begin); break;
        }
    }

    void AmbientOcclusion::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Ambient Occlusion", ImGuiTreeNodeFlags_Framed)) {
            const char* modes[] = { "None", "SSAO (UNIMPLEMENTED)", "GTAO (UNIMPLEMENTED)", "RTAO (UNIMPLEMENTED)" };
            if (ImGui::BeginCombo("Technique", modes[(int)mMode])) {
                for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
                    bool disabled = false;
                
                    // Disable RT modes if device doesn’t support raytracing
                    if ((i == (int)AmbientOcclusionMode::kRTAO)
                        && !Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                        disabled = true;
                    }
                
                    if (disabled) {
                        ImGui::BeginDisabled();
                        ImGui::Selectable(modes[i], false);
                        ImGui::EndDisabled();
                    } else {
                        bool isSelected = (mMode == (AmbientOcclusionMode)i);
                        if (ImGui::Selectable(modes[i], isSelected)) {
                            mMode = (AmbientOcclusionMode)i;
                        }
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::TreePop();
        }
    }

    void AmbientOcclusion::None(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::AmbientOcclusion::None");
        Gfx::Resource& before = Gfx::ResourceManager::Import(AMBIENT_OCCLUSION_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(1.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void AmbientOcclusion::SSAO(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::AmbientOcclusion::SSAO");
        Gfx::Resource& before = Gfx::ResourceManager::Import(AMBIENT_OCCLUSION_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(1.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void AmbientOcclusion::GTAO(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::AmbientOcclusion::GTAO");
        Gfx::Resource& before = Gfx::ResourceManager::Import(AMBIENT_OCCLUSION_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(1.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void AmbientOcclusion::RTAO(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::AmbientOcclusion::RTAO");
        Gfx::Resource& before = Gfx::ResourceManager::Import(AMBIENT_OCCLUSION_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(1.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }
}