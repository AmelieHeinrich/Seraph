//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#include "SP_IndirectDiffuse.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ViewRecycler.h>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace SP
{
    IndirectDiffuse::IndirectDiffuse()
    {
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        // Texture
        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;

        Gfx::ResourceManager::CreateTexture(INDIRECT_DIFFUSE_MASK_ID, hdrDesc);
    }

    IndirectDiffuse::~IndirectDiffuse()
    {

    }

    void IndirectDiffuse::Render(RenderPassBegin& begin)
    {
        switch (mMode)
        {
            case IndirectDiffuseMode::kNone: None(begin); break;
            case IndirectDiffuseMode::kBaked: Baked(begin); break;
            case IndirectDiffuseMode::kConstantAmbient: ConstantAmbient(begin); break;
            case IndirectDiffuseMode::kSSGI: SSGI(begin); break;
            case IndirectDiffuseMode::kDDGI: DDGI(begin); break;
        }
    }

    void IndirectDiffuse::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Global Illumination (Indirect Diffuse)", ImGuiTreeNodeFlags_Framed)) {
            const char* modes[] = { "None", "Constant Ambient", "Baked (UNIMPLEMENTED)", "SSGI (UNIMPLEMENTED)", "DDGI (UNIMPLEMENTED)" };
            if (ImGui::BeginCombo("Technique", modes[(int)mMode])) {
                for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
                    bool disabled = false;
                
                    // Disable RT modes if device doesn’t support raytracing
                    if ((i == (int)IndirectDiffuseMode::kDDGI)
                        && !Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                        disabled = true;
                    }
                
                    if (disabled) {
                        ImGui::BeginDisabled();
                        ImGui::Selectable(modes[i], false);
                        ImGui::EndDisabled();
                    } else {
                        bool isSelected = (mMode == (IndirectDiffuseMode)i);
                        if (ImGui::Selectable(modes[i], isSelected)) {
                            mMode = (IndirectDiffuseMode)i;
                        }
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (mMode == IndirectDiffuseMode::kConstantAmbient) {
                ImGui::ColorEdit3("Ambient Color", glm::value_ptr(mConstantAmbient), ImGuiColorEditFlags_DisplayHSV);
            }

            ImGui::TreePop();
        }
    }

    void IndirectDiffuse::None(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectDiffuse::None");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectDiffuse::ConstantAmbient(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectDiffuse::ConstantAmbient");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, mConstantAmbient);
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectDiffuse::Baked(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectDiffuse::Baked");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, mConstantAmbient);
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectDiffuse::SSGI(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectDiffuse::SSGI");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void IndirectDiffuse::DDGI(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectDiffuse::DDGI");
        Gfx::Resource& before = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(0.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }
}
