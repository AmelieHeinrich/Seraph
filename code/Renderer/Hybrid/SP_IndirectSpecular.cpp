//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#include "SP_IndirectSpecular.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ViewRecycler.h>
#include <imgui.h>

namespace SP
{
    IndirectSpecular::IndirectSpecular()
    {
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        // Texture
        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;

        Gfx::ResourceManager::CreateTexture(INDIRECT_SPECULAR_MASK_ID, hdrDesc);
    }

    IndirectSpecular::~IndirectSpecular()
    {

    }

    void IndirectSpecular::Render(RenderPassBegin& begin)
    {
        switch (mMode)
        {
            case IndirectSpecularMode::kNone: None(begin); break;
            case IndirectSpecularMode::kScreenSpace: ScreenSpace(begin); break;
            case IndirectSpecularMode::kRaytraced: Raytrace(begin); break;
            case IndirectSpecularMode::kHybrid: Hybrid(begin); break;
        }
    }

    void IndirectSpecular::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Reflections (Indirect Specular)", ImGuiTreeNodeFlags_Framed)) {
            const char* modes[] = { "None", "SSR (UNIMPLEMENTED)", "Raytraced (UNIMPLEMENTED)", "Hybrid (UNIMPLEMENTED)" };
            if (ImGui::BeginCombo("Technique", modes[(int)mMode])) {
                for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
                    bool disabled = false;
                
                    // Disable RT modes if device doesn’t support raytracing
                    if ((i == (int)IndirectSpecularMode::kRaytraced || i == (int)IndirectSpecularMode::kHybrid)
                        && !Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                        disabled = true;
                    }
                
                    if (disabled) {
                        ImGui::BeginDisabled();
                        ImGui::Selectable(modes[i], false);
                        ImGui::EndDisabled();
                    } else {
                        bool isSelected = (mMode == (IndirectSpecularMode)i);
                        if (ImGui::Selectable(modes[i], isSelected)) {
                            mMode = (IndirectSpecularMode)i;
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

    void IndirectSpecular::None(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::None");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectSpecular::ScreenSpace(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::ScreenSpace");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectSpecular::Raytrace(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Raytrace");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectSpecular::Hybrid(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Hybrid");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }
}
