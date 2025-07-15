//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-14 20:25:23
//

#include "VisualizeMotionVectors.h"
#include "Tonemapping.h"
#include "GBuffer.h"

#include <ImGui/imgui.h>

VisualizeMotionVectors::VisualizeMotionVectors(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    RHIComputePipelineDesc desc = {};
    desc.PushConstantSize = sizeof(uint) * 4;

    PipelineReloader::SubscribeCompute("VisualizeMotionVectors.hlsl", desc, "CSMain");
}

VisualizeMotionVectors::~VisualizeMotionVectors()
{
}

void VisualizeMotionVectors::Render(RenderPassBegin& begin)
{
    if (!mEnable)
        return;

    begin.CommandList->PushMarker("Visualize Motion Vectors");
    CODE_BLOCK("Render") {
        RendererResource& ldr = RendererResourceManager::Import(TONEMAPPING_LDR_ID, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& motionVec = RendererResourceManager::Import(GBUFFER_MOTION_VECTOR_ID, begin.CommandList, RendererImportType::kShaderRead);

        struct PushConstants {
            BindlessHandle MotionVector;
            BindlessHandle Output;
            uint Width;
            uint Height;
        } constants = {
            RendererViewRecycler::GetSRV(motionVec.Texture)->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(ldr.Texture)->GetBindlessHandle(),
            mWidth,
            mHeight
        };

        IRHIComputePipeline* pipeline = PipelineReloader::GetCompute("VisualizeMotionVectors.hlsl");
        begin.CommandList->SetComputePipeline(pipeline);
        begin.CommandList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}

void VisualizeMotionVectors::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Motion Vector Visualizer", ImGuiTreeNodeFlags_Framed)) {
        ImGui::Checkbox("Enable", &mEnable);
        ImGui::TreePop();
    }   
}
