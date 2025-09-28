//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#include "SP_IndirectSpecular.h"
#include "SP_IndirectDiffuse.h"
#include "SP_Application.h"
#include "SP_GBuffer.h"

#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_ShaderManager.h>
#include <imgui.h>
#include <ToolDevConsole/TDC_Console.h>

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

        // Modes
        CODE_BLOCK("Baked") {
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/indirect_specular/baked/skybox_bake.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/indirect_specular/baked/brdf_bake.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/indirect_specular/baked/populate_mask.kds");

            KGPU::TextureDesc bakedCubemapDesc;
            bakedCubemapDesc.Width = 1024;
            bakedCubemapDesc.Height = 1024;
            bakedCubemapDesc.Depth = 6;
            bakedCubemapDesc.MipLevels = 5;
            bakedCubemapDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
            bakedCubemapDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage;

            KGPU::TextureDesc brdfDesc;
            brdfDesc.Width = 512;
            brdfDesc.Height = 512;
            brdfDesc.Format = KGPU::TextureFormat::kR16G16_FLOAT;
            brdfDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage;

            Gfx::ResourceManager::CreateTexture(INDIRECT_SPECULAR_BAKED_CUBEMAP_ID, bakedCubemapDesc);
            Gfx::ResourceManager::CreateTexture(INDIRECT_SPECULAR_BAKED_BRDF_ID, brdfDesc);

            TDC::Console::AddFunction("Graphics.Reflections.RebakeSkybox", [&](const KC::String&) {
                mSkyboxPath = "";
            });
        }
    }

    IndirectSpecular::~IndirectSpecular()
    {
    }

    void IndirectSpecular::Render(RenderPassBegin& begin)
    {
        switch (mMode)
        {
            case IndirectSpecularMode::kNone: None(begin); break;
            case IndirectSpecularMode::kBaked: Baked(begin); break;
            case IndirectSpecularMode::kScreenSpace: ScreenSpace(begin); break;
            case IndirectSpecularMode::kRaytraced: Raytrace(begin); break;
            case IndirectSpecularMode::kHybrid: Hybrid(begin); break;
        }
    }

    void IndirectSpecular::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Reflections (Indirect Specular)", ImGuiTreeNodeFlags_Framed)) {
            const char* modes[] = { "None", "Baked", "SSR (UNIMPLEMENTED)", "Raytraced (UNIMPLEMENTED)", "Hybrid (UNIMPLEMENTED)" };
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

    void IndirectSpecular::Baked(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Baked");

        CODE_BLOCK("Bake BRDF") {
            if (!mBakedBRDF) {
                KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Baked(BakeBRDF)");

                Gfx::Resource& brdf = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_BAKED_BRDF_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
                uint size = brdf.Texture->GetDesc().Width;

                struct Constants {
                    KGPU::BindlessHandle View;
                    float3 Pad;
                } constants = {
                    Gfx::ViewRecycler::GetUAV(brdf.Texture)->GetBindlessHandle()
                };

                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/indirect_specular/baked/brdf_bake.kds");
                begin.CmdList->BeginCompute();
                begin.CmdList->SetComputePipeline(pipeline);
                begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
                begin.CmdList->Dispatch(KGPU::uint3(size / 32, size / 32, 1), KGPU::uint3(32, 32, 1));
                begin.CmdList->EndCompute();
                mBakedBRDF = true;
            }
        }

        CODE_BLOCK("Bake skybox") {
            if (mSkyboxPath != begin.Sky->Path) {
                KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Baked(BakeCubemap)");
                mSkyboxPath = begin.Sky->Path;

                Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
                Gfx::Resource& baked = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_BAKED_CUBEMAP_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
                uint size = baked.Texture->GetDesc().Width;
                uint mips = baked.Texture->GetDesc().MipLevels;

                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/indirect_specular/baked/skybox_bake.kds");
                begin.CmdList->BeginCompute();
                begin.CmdList->SetComputePipeline(pipeline);

                const float deltaRoughness = 1.0f / std::max((float)baked.Texture->GetDesc().MipLevels - 1u, 1.0f);
                for (int i = 0, x = size; i < mips; i++, x /= 2) {
                    const uint numGroups = std::max(1u, x / 32u);

                    KGPU::TextureViewDesc viewDesc;
                    viewDesc.Texture = baked.Texture;
                    viewDesc.Type = KGPU::TextureViewType::kShaderWrite;
                    viewDesc.ViewMip = i;
                    viewDesc.ArrayLayer = VIEW_ALL_MIPS;
                    viewDesc.Dimension = KGPU::TextureViewDimension::kTextureCube;
                    viewDesc.ViewFormat = KGPU::TextureFormat::kR16G16B16A16_FLOAT;

                    struct PushConstant {
                        KGPU::BindlessHandle EnvMap;
                        KGPU::BindlessHandle BakedMip;
                        KGPU::BindlessHandle Sampler;
                        float Roughness;
                    } constants = {
                        begin.Sky->CubeView->GetBindlessHandle(),
                        Gfx::ViewRecycler::GetTextureView(viewDesc)->GetBindlessHandle(),
                        sampler.Sampler->GetBindlessHandle(),
                        i * deltaRoughness
                    };

                    begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
                    begin.CmdList->Dispatch(KGPU::uint3(numGroups, numGroups, 6), KGPU::uint3(32, 32, 1));
                }
                begin.CmdList->EndCompute();
            }
        }

        CODE_BLOCK("Populate mask") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::IndirectSpecular::Baked(PopulateMask)");

            Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
            Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
            Gfx::Resource& gbufferDepth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& gbufferColor = Gfx::ResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& gbufferNormal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& gbufferPBR = Gfx::ResourceManager::Import(GBUFFER_PBR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& brdf = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_BAKED_BRDF_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& baked = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_BAKED_CUBEMAP_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& diffuse = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_BAKED_IRRADIANCE_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& output = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);

            KGPU::TextureViewDesc cubeViewDesc;
            cubeViewDesc.Texture = baked.Texture;
            cubeViewDesc.ArrayLayer = KGPU::VIEW_ALL_MIPS;
            cubeViewDesc.Dimension = KGPU::TextureViewDimension::kTextureCube;
            cubeViewDesc.ViewFormat = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
            cubeViewDesc.Type = KGPU::TextureViewType::kShaderRead;
            cubeViewDesc.ViewMip = KGPU::VIEW_ALL_MIPS;

            KGPU::TextureViewDesc diffuseIblDesc;
            diffuseIblDesc.Texture = diffuse.Texture;
            diffuseIblDesc.ArrayLayer = KGPU::VIEW_ALL_MIPS;
            diffuseIblDesc.Dimension = KGPU::TextureViewDimension::kTextureCube;
            diffuseIblDesc.ViewFormat = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
            diffuseIblDesc.Type = KGPU::TextureViewType::kShaderRead;

            struct PushConstants {
                KGPU::BindlessHandle BakedCubemap;
                KGPU::BindlessHandle BRDF;
                KGPU::BindlessHandle Sampler;
                KGPU::BindlessHandle GBufferColor;

                KGPU::BindlessHandle GBufferPBR;
                KGPU::BindlessHandle GBufferDepth;
                int Width;
                int Height;

                KGPU::BindlessHandle Output;
                KGPU::BindlessHandle CameraBuffer;
                KGPU::BindlessHandle GBufferNormal;
                KGPU::BindlessHandle BakedDiffuse;
            } constants = {
                Gfx::ViewRecycler::GetTextureView(cubeViewDesc)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(brdf.Texture)->GetBindlessHandle(),
                sampler.Sampler->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(gbufferColor.Texture)->GetBindlessHandle(),

                Gfx::ViewRecycler::GetSRV(gbufferPBR.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(gbufferDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
                begin.Width,
                begin.Height,

                Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
                cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetTextureView(diffuseIblDesc)->GetBindlessHandle()
            };

            auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/indirect_specular/baked/populate_mask.kds");
            begin.CmdList->BeginCompute();
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch(KGPU::uint3((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1), KGPU::uint3(8, 8, 1));
            begin.CmdList->EndCompute();
        }
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
