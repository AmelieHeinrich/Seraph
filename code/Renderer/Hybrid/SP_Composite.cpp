//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 14:13:52
//

#include "SP_Composite.h"
#include "SP_Tonemap.h"
#include "SP_GBuffer.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_Manager.h>

namespace SP
{
    Composite::Composite()
    {
        KGPU::GraphicsPipelineDesc desc;
        desc.RenderTargetFormats.push_back(Gfx::Manager::GetDevice()->GetSurfaceFormat());

        Gfx::ShaderManager::SubscribeGraphics("data/sp/shaders/post_fx/render_texture.kds", desc);
    }

    void Composite::Render(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Composite::Render");
        KGPU::TextureBarrier swapchainBarrier(begin.SwapTexture);
        swapchainBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
        swapchainBarrier.DestAccess = KGPU::ResourceAccess::kColorAttachmentWrite;
        swapchainBarrier.SourceStage = KGPU::PipelineStage::kNone;
        swapchainBarrier.DestStage = KGPU::PipelineStage::kColorAttachmentOutput;
        swapchainBarrier.NewLayout = KGPU::ResourceLayout::kColorAttachment;

        KGPU::RenderBegin renderBegin(begin.Width, begin.Height, { KGPU::RenderAttachment(begin.SwapView, false) }, {});

        Gfx::Resource& ldr = Gfx::ResourceManager::Import(TONEMAPPING_LDR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);
        KGPU::IGraphicsPipeline* pipeline = Gfx::ShaderManager::GetGraphics("data/sp/shaders/post_fx/render_texture.kds");

        struct Constants {
            KGPU::BindlessHandle in;
            KGPU::BindlessHandle sampler;
            KGPU::uint2 Pad;
        } constants = {
            Gfx::ViewRecycler::GetSRV(ldr.Texture)->GetBindlessHandle(),
            sampler.Sampler->GetBindlessHandle(),
            {}
        };

        begin.CmdList->Barrier(swapchainBarrier);
        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->SetGraphicsPipeline(pipeline);
        begin.CmdList->SetRenderSize(begin.Width, begin.Height);
        begin.CmdList->SetGraphicsConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Draw(3, 1, 0, 0);
        begin.CmdList->EndRendering();
    }
}