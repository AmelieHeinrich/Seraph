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
    
}

void VisualizeMotionVectors::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Motion Vector Visualizer", ImGuiTreeNodeFlags_Framed)) {
        ImGui::Checkbox("Enable", &mEnable);
        ImGui::TreePop();
    }   
}
