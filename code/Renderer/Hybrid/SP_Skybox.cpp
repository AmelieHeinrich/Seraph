//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 15:49:53
//

#include "SP_Skybox.h"
#include "SP_GBuffer.h"
#include "SP_Radiance.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <ToolDevConsole/TDC_Console.h>

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

namespace SP
{
    Skybox::Skybox()
    {
        TDC::Console::AddVariable("Graphics.Skybox.Enable", mEnable);

        KGPU::GraphicsPipelineDesc desc;
        desc.CullMode = KGPU::CullMode::kNone;
        desc.DepthEnabled = true;
        desc.DepthWrite = false;
        desc.DepthClampEnabled = true;
        desc.DepthOperation = KGPU::DepthOperation::kLessEqual;
        desc.RenderTargetFormats.push_back(KGPU::TextureFormat::kR16G16B16A16_FLOAT);

        Gfx::ShaderManager::SubscribeGraphics("data/sp/shaders/sky.kds", desc);
    }

    void Skybox::Render(RenderPassBegin& begin)
    {
        if (!mEnable)
            return;

        KGPU::ScopedMarker _(begin.CmdList, "SP::Skybox::Render");

        Gfx::Resource& depthTexture = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kDepthWrite);
        Gfx::Resource& hdr = Gfx::ResourceManager::Import(RADIANCE_HDR_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);
        
        KGPU::RenderBegin renderBegin(begin.Width, begin.Height, { KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(hdr.Texture), false) }, KGPU::RenderAttachment(Gfx::ViewRecycler::GetDSV(depthTexture.Texture), false));
        KGPU::IGraphicsPipeline* pipeline = Gfx::ShaderManager::GetGraphics("data/sp/shaders/sky.kds");

        struct PushConstants {
            KGPU::BindlessHandle EnvMap;
            KGPU::BindlessHandle Sampler;
            KGPU::uint2 Pad;
        
            KGPU::float4x4 ModelViewProjection;
        } constants = {
            begin.Sky->CubeView->GetBindlessHandle(),
            sampler.Sampler->GetBindlessHandle(),
            {},

            begin.CamData.Proj * KGPU::float4x4(KGPU::float3x3(begin.CamData.View)) * glm::scale(glm::mat4(1.0f), glm::vec3(10000.0f))
        };

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->SetRenderSize(begin.Width, begin.Height);
        begin.CmdList->SetGraphicsPipeline(pipeline);
        begin.CmdList->SetGraphicsConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Draw(36, 1, 0, 0);
        begin.CmdList->EndRendering();
    }

    void Skybox::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Skybox", ImGuiTreeNodeFlags_Framed)) {
            ImGui::Checkbox("Enabled", &mEnable);
            ImGui::TreePop();
        }
    }
}
