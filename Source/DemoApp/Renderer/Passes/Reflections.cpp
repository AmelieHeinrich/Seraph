//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-23 20:09:01
//

#include "Reflections.h"

#include <imgui/imgui.h>

Reflections::Reflections(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    RHITextureDesc reflectionDesc;
    reflectionDesc.Width = width;
    reflectionDesc.Height = height;
    reflectionDesc.Format = RHITextureFormat::kR16G16B16A16_FLOAT;
    reflectionDesc.MipLevels = 1;
    reflectionDesc.Usage = RHITextureUsage::kShaderResource | RHITextureUsage::kStorage | RHITextureUsage::kRenderTarget;
    
    RendererResourceManager::CreateTexture(REFLECTIONS_CHANNEL_ID, reflectionDesc);
    RendererResourceManager::CreateTexture(REFLECTIONS_LIGHTING_COPY_ID, reflectionDesc);
}

Reflections::~Reflections()
{

}

void Reflections::Render(RenderPassBegin& begin)
{
    switch (mMode) {
        case ReflectionsMode::kNone: {
            None(begin);
            break;
        }
        case ReflectionsMode::kScreenSpace: {
            ScreenSpace(begin);
            break;
        }
        case ReflectionsMode::kRayTrace: {
            RayTrace(begin);
            break;
        }
        case ReflectionsMode::kHybrid: {
            Hybrid(begin);
            break;
        }
    }
}

void Reflections::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Reflections", ImGuiTreeNodeFlags_Framed)) {
        const char* modes[] = { "None", "Screen-space", "Ray-trace", "Hybrid SS+RT" };
        ImGui::Combo("Technique", (int*)&mMode, modes, 4, 4);

        ImGui::TreePop();
    }
}

void Reflections::None(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("No Reflections");
    CODE_BLOCK("Execute") {
        RendererResource& mask = RendererResourceManager::Import(REFLECTIONS_CHANNEL_ID, begin.CommandList, RendererImportType::kColorWrite);

        RHIRenderAttachment attachment(RendererViewRecycler::GetRTV(mask.Texture), true);
        RHIRenderBegin renderBegin(mask.Texture->GetDesc().Width, mask.Texture->GetDesc().Height, { attachment }, {});

        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}

void Reflections::ScreenSpace(RenderPassBegin& begin)
{
    None(begin);
}

void Reflections::RayTrace(RenderPassBegin& begin)
{
    None(begin);
}

void Reflections::Hybrid(RenderPassBegin& begin)
{
    None(begin);
}
