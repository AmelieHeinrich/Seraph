//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 22:36:22
//

#include "SP_MotionVector.h"
#include "SP_Tonemap.h"
#include "SP_GBuffer.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <imgui.h>

namespace SP
{
    MotionVector::MotionVector()
    {
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/motion_vectors.kds");
    }

    void MotionVector::Render(RenderPassBegin& begin)
    {
        if (!mEnable)
            return;

        KGPU::ScopedMarker _(begin.CmdList, "SP::MotionVector::Render");

        Gfx::Resource& ldr = Gfx::ResourceManager::Import(TONEMAPPING_LDR_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& motionVec = Gfx::ResourceManager::Import(GBUFFER_MOTION_VECTOR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);

        struct PushConstants {
            BindlessHandle MotionVector;
            BindlessHandle Output;
            int Width;
            int Height;
        } constants = {
            Gfx::ViewRecycler::GetSRV(motionVec.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(ldr.Texture)->GetBindlessHandle(),
            begin.Width,
            begin.Height
        };

        KGPU::IComputePipeline* pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/motion_vectors.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
    }

    void MotionVector::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Motion Vector Visualizer", ImGuiTreeNodeFlags_Framed)) {
            ImGui::Checkbox("Enable", &mEnable);
            ImGui::TreePop();
        }   
    }
}
