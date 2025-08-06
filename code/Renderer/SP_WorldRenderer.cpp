//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:56:00
//

#include "SP_WorldRenderer.h"

#include "Hybrid/SP_GBuffer.h"
#include "Hybrid/SP_Shadows.h"
#include "Hybrid/SP_LightCull.h"
#include "Hybrid/SP_Deferred.h"
#include "Hybrid/SP_Skybox.h"
#include "Hybrid/SP_Tonemap.h"
#include "Hybrid/SP_Debug.h"
#include "Hybrid/SP_MotionVector.h"
#include "Hybrid/SP_Composite.h"

#include <Graphics/Gfx_TempResourceStorage.h>
#include <Graphics/Gfx_CommandListRecycler.h>
#include <Graphics/Gfx_ShaderManager.h>

#include <imgui.h>

namespace SP
{
    WorldRenderer::WorldRenderer()
    {
        mPasses = {
            KC_NEW(GBuffer),
            KC_NEW(Shadows),
            KC_NEW(LightCulling),
            KC_NEW(Deferred),
            KC_NEW(Skybox),
            KC_NEW(Tonemap),
            KC_NEW(Debug),
            KC_NEW(MotionVector),
            KC_NEW(Composite)
        };
    }

    WorldRenderer::~WorldRenderer()
    {
        for (auto& pass : mPasses) {
            KC_DELETE(pass);
        }
        mPasses.clear();
    }

    void WorldRenderer::Render(RenderPassBegin& begin)
    {
        begin.World->GetLightList()->Update(begin.FrameIndex);

        for (auto& pass : mPasses) {
            pass->Render(begin);
        }
    }

    void WorldRenderer::UI(RenderPassBegin& begin)
    {
        ImGui::Begin("Renderer Settings");
        for (auto& pass : mPasses) {
            pass->UI(begin);
        }
        ImGui::End();
    }

    void WorldRenderer::Prepare()
    {
        Gfx::CommandListRecycler::FlushCommandLists();
        Gfx::TempResourceStorage::Clean();
    }
}
