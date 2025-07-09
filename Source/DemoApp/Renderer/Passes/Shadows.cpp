//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:29:24
//

#include "Shadows.h"
#include "GBuffer.h"

#include <imgui/imgui.h>

Shadows::Shadows(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Create mask
    RHITextureDesc sunMaskDesc;
    sunMaskDesc.Width = width;
    sunMaskDesc.Height = height;
    sunMaskDesc.Usage = RHITextureUsage::kShaderResource | RHITextureUsage::kRenderTarget | RHITextureUsage::kStorage;
    sunMaskDesc.Format = RHITextureFormat::kR32_FLOAT;
    
    RendererResourceManager::CreateTexture(SHADOWS_SUN_MASK_ID, sunMaskDesc);

    // Create pipelines
    CODE_BLOCK("Create Hard RT resources") {
        CompiledShader shader = ShaderCompiler::Compile("Shadows/HardRT", { "CSMain" });
        CompiledShader shaderNoAlpha = ShaderCompiler::Compile("Shadows/HardRTNoAlpha", { "CSMain" });

        mHardRTShadows = mParentDevice->CreateComputePipeline(RHIComputePipelineDesc(sizeof(uint) * 12, shader.Entries["CSMain"]));
        mHardRTShadowsNoAlpha = mParentDevice->CreateComputePipeline(RHIComputePipelineDesc(sizeof(uint) * 12, shaderNoAlpha.Entries["CSMain"]));
    }
}

Shadows::~Shadows()
{
    delete mHardRTShadows;
}

void Shadows::Render(RenderPassBegin& begin)
{
    switch (mMode) {
        case ShadowMode::kNone: {
            None(begin);
            break;
        }
        case ShadowMode::kHardRT: {
            HardRT(begin);
            break;
        }
        default: {
            SERAPH_WARN("SHADOW TECHNIQUE NOT IMPLEMENTED YET!");
            break;
        }
    }
}

void Shadows::None(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("No Shadows");
    CODE_BLOCK("Execute") {
        RendererResource& before = RendererResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CommandList, RendererImportType::kColorWrite);

        RHIRenderAttachment attachment(RendererViewRecycler::GetRTV(before.Texture), true, float3(1.0f));
        RHIRenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}

void Shadows::HardRT(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Hard RT Shadows");
    CODE_BLOCK("Execute") {
        RendererResource& output = RendererResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& gbufferDepth = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& gbufferNormal = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        RendererResource& materialSampler = RendererResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);

        struct PushConstants {
            BindlessHandle SunArray;
            BindlessHandle Output;
            BindlessHandle AS;
            float NormalBias;

            uint Width;
            uint Height;
            BindlessHandle Depth;
            BindlessHandle Normal;

            BindlessHandle Camera;
            BindlessHandle Sampler;
            BindlessHandle Instances;
            uint Pad;
        } constants = {
            begin.RenderScene->GetLights().GetSunBufferView(begin.FrameIndex)->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            begin.RenderScene->GetTLAS()->GetBindlessHandle(),
            mNormalBias,

            mWidth,
            mHeight,
            RendererViewRecycler::GetTextureView(RHITextureViewDesc(gbufferDepth.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),

            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            materialSampler.Sampler->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(begin.RenderScene->GetSceneInstanceBuffer())->GetBindlessHandle(),
            0
        };

        IRHIComputePipeline* pipeline = mAlphaTest ? mHardRTShadows : mHardRTShadowsNoAlpha;
        begin.CommandList->SetComputePipeline(pipeline);
        begin.CommandList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);
    }
    begin.CommandList->PopMarker();
}

void Shadows::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Shadows", ImGuiTreeNodeFlags_Framed)) {
        const char* modes[] = { "None", "CSM", "Hard RT", "Soft RT" };
        ImGui::Combo("Shadow Technique", (int*)&mMode, modes, 4, 4);
        ImGui::Checkbox("Alpha Test", &mAlphaTest);

        switch (mMode) {
            case ShadowMode::kHardRT: {
                ImGui::SliderFloat("Normal Bias", &mNormalBias, 0.001f, 0.01f);
                break;
            }
            default: {
                break;
            }
        }
        ImGui::TreePop();
    }
}
