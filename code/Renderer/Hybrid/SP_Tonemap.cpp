//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 14:01:09
//

#include "SP_Tonemap.h"
#include "SP_Radiance.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_Screenshotter.h>

namespace SP
{
    Tonemap::Tonemap()
    {
        // Texture
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.Format = KGPU::TextureFormat::kR8G8B8A8_UNORM;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;
        
        Gfx::ResourceManager::CreateTexture(TONEMAPPING_LDR_ID, hdrDesc);
        Gfx::ResourceManager::CreateTexture(TONEMAPPING_SCREENSHOT_ID, hdrDesc);

        // Pipeline
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/post_fx/tonemap.kds");
    }

    void Tonemap::Render(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Tonemap::Render");
        Gfx::Resource& hdr = Gfx::ResourceManager::Import(RADIANCE_HDR_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& ldr = Gfx::ResourceManager::Import(TONEMAPPING_LDR_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);

        struct Constants {
            BindlessHandle hdrHandle;
            BindlessHandle ldrHandle;
            int width;
            int height;
        } constants = {
            Gfx::ViewRecycler::GetSRV(hdr.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(ldr.Texture)->GetBindlessHandle(),
            begin.Width,
            begin.Height
        };

        KGPU::IComputePipeline* pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/tonemap.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);

        Gfx::Resource& src = Gfx::ResourceManager::Import(TONEMAPPING_LDR_ID, begin.CmdList, Gfx::ImportType::kTransferSource);
        Gfx::Resource& dst = Gfx::ResourceManager::Import(TONEMAPPING_SCREENSHOT_ID, begin.CmdList, Gfx::ImportType::kTransferDest);
        
        begin.CmdList->CopyTextureToTexture(dst.Texture, src.Texture);
    }
}
