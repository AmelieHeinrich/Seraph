//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-23 21:27:49
//

#include "SP_Debug.h"
#include "SP_Tonemap.h"

#include <Effects/FX_DebugRenderer.h>

namespace SP
{
    Debug::Debug()
    {
        FX::DebugRenderer::Initialize();
    }

    void Debug::Render(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker marker(begin.CmdList, "SP::Debug::Render");

        Gfx::Resource& render = Gfx::ResourceManager::Import(TONEMAPPING_LDR_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        
        FX::DebugRendererDesc desc = {
            .CommandList = begin.CmdList,
            .RenderTexture = render.Texture,
            .Projection = begin.CamData.Proj,
            .View = begin.CamData.View
        };
        FX::DebugRenderer::Render(desc);
    }
}
