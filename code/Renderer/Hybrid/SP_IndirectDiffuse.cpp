//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#include "SP_IndirectDiffuse.h"
#include "SP_Application.h"
#include "SP_GBuffer.h"

#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_ShaderManager.h>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <ToolDevConsole/TDC_Console.h>

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

        CODE_BLOCK("Baked") {
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/indirect_diffuse/baked/irradiance_bake.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/indirect_diffuse/baked/populate_mask.kds");

            KGPU::TextureDesc bakedCubemapDesc;
            bakedCubemapDesc.Width = 32;
            bakedCubemapDesc.Height = 32;
            bakedCubemapDesc.Depth = 6;
            bakedCubemapDesc.MipLevels = 1;
            bakedCubemapDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
            bakedCubemapDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage;

            Gfx::ResourceManager::CreateTexture(INDIRECT_DIFFUSE_BAKED_IRRADIANCE_ID, bakedCubemapDesc);

            TDC::Console::AddFunction("Graphics.GI.RebakeSkybox", [&](const KC::String&) {
                mCurrentSkybox = nullptr;
            });
        }
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
            const char* modes[] = { "None", "Constant Ambient", "Baked", "SSGI (UNIMPLEMENTED)", "DDGI (UNIMPLEMENTED)" };
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
        
        CODE_BLOCK("Bake Irradiance Map") {
            if (mCurrentSkybox != begin.Sky) {
                KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Baked(BakeSkybox)");
                mCurrentSkybox = begin.Sky;

                Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
                Gfx::Resource& baked = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_BAKED_IRRADIANCE_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
            
                KGPU::TextureViewDesc viewDesc(baked.Texture, KGPU::TextureViewType::kShaderWrite, KGPU::TextureFormat::kR16G16B16A16_FLOAT);
                viewDesc.Dimension = KGPU::TextureViewDimension::kTextureCube;

                struct PushConstants {
                    KGPU::BindlessHandle EnvironmentMap;
                    KGPU::BindlessHandle IrradianceMap;
                    KGPU::BindlessHandle CubeSampler;
                    uint _Pad0;
                } constants = {
                    begin.Sky->CubeView->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(viewDesc)->GetBindlessHandle(),
                    sampler.Sampler->GetBindlessHandle(),
                    0
                };
                
                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/indirect_diffuse/baked/irradiance_bake.kds");
                begin.CmdList->SetComputePipeline(pipeline);
                begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
                begin.CmdList->Dispatch(1, 1, 6);
            }
        }

        CODE_BLOCK("Populate Mask") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Baked(PopulateMask)");

            Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
            Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
            Gfx::Resource& gbufferColor = Gfx::ResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& gbufferNormal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& gbufferPBR = Gfx::ResourceManager::Import(GBUFFER_PBR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& gbufferDepth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& baked = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_BAKED_IRRADIANCE_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& output = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);

            KGPU::TextureViewDesc cubeViewDesc;
            cubeViewDesc.Texture = baked.Texture;
            cubeViewDesc.ArrayLayer = KGPU::VIEW_ALL_MIPS;
            cubeViewDesc.Dimension = KGPU::TextureViewDimension::kTextureCube;
            cubeViewDesc.ViewFormat = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
            cubeViewDesc.Type = KGPU::TextureViewType::kShaderRead;

            struct PushConstants {
                KGPU::BindlessHandle GBufferDepth;
                KGPU::BindlessHandle GBufferColor;
                KGPU::BindlessHandle GBufferPBR;
                KGPU::BindlessHandle GBufferNormal;
                
                KGPU::BindlessHandle Cubemap;
                KGPU::BindlessHandle Sampler;
                KGPU::BindlessHandle Output;
                int Width;

                int Height;
                KGPU::BindlessHandle CameraBuffer;
                uint2 Pad;
            } constants = {
                Gfx::ViewRecycler::GetSRV(gbufferColor.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(gbufferPBR.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(gbufferDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),

                Gfx::ViewRecycler::GetTextureView(cubeViewDesc)->GetBindlessHandle(),
                sampler.Sampler->GetBindlessHandle(),
                Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
                begin.Width,

                begin.Height,
                cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
                {}
            };

            auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/indirect_diffuse/baked/populate_mask.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        }
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
