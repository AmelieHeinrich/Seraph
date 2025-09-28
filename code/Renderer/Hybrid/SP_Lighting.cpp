//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-08-19 11:54:00
//

#include "SP_Lighting.h"
#include "SP_Application.h"
#include "SP_GBuffer.h"
#include "SP_Radiance.h"
#include "SP_IndirectDiffuse.h"
#include "SP_IndirectSpecular.h"
#include "SP_AmbientOcclusion.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <imgui.h>

namespace SP
{
    Lighting::Lighting()
    {
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        // Texture
        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;

        Gfx::ResourceManager::CreateTexture(LIGHTING_OUTPUT_ID, hdrDesc);
        
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/lighting.kds");
    }

    Lighting::~Lighting()
    {
    }

    void Lighting::Render(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Lighting::Render");

        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        Gfx::Resource& gbufferDepth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferColor = Gfx::ResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferNormal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferPBR = Gfx::ResourceManager::Import(GBUFFER_PBR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferEmissive = Gfx::ResourceManager::Import(GBUFFER_EMISSIVE_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& directDiffuse = Gfx::ResourceManager::Import(RADIANCE_DIRECT_DIFFUSE_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& directSpecular = Gfx::ResourceManager::Import(RADIANCE_DIRECT_SPECULAR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& indirectDiffuse = Gfx::ResourceManager::Import(INDIRECT_DIFFUSE_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& indirectSpecular = Gfx::ResourceManager::Import(INDIRECT_SPECULAR_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& ao = Gfx::ResourceManager::Import(AMBIENT_OCCLUSION_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& output = Gfx::ResourceManager::Import(LIGHTING_OUTPUT_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);

        struct PushConstants {
            KGPU::BindlessHandle GBufferDepth;
            KGPU::BindlessHandle GBufferColor;
            KGPU::BindlessHandle GBufferNormal;
            KGPU::BindlessHandle GBufferPBR;
        
            KGPU::BindlessHandle DirectDiffuse;
            KGPU::BindlessHandle DirectSpecular;
            KGPU::BindlessHandle IndirectDiffuse;
            KGPU::BindlessHandle IndirectSpecular;
        
            KGPU::BindlessHandle AO;
            KGPU::BindlessHandle CameraBuffer;
            KGPU::BindlessHandle Output;
            KGPU::BindlessHandle GBufferEmissive;

            int Width;
            int Height;
            uint Pad2[2];
        } constants = {
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(gbufferDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferColor.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferPBR.Texture)->GetBindlessHandle(),

            Gfx::ViewRecycler::GetSRV(directDiffuse.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(directSpecular.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(indirectDiffuse.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(indirectSpecular.Texture)->GetBindlessHandle(),

            Gfx::ViewRecycler::GetSRV(ao.Texture)->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferEmissive.Texture)->GetBindlessHandle(),

            begin.Width,
            begin.Height
        };

        auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/lighting.kds");
        begin.CmdList->BeginCompute();
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch(KGPU::uint3((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1), KGPU::uint3(8, 8, 1));
        begin.CmdList->EndCompute();
    }

    void Lighting::UI(RenderPassBegin& begin)
    {
    }
}
