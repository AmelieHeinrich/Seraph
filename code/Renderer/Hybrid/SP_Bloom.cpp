//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-08-20 14:16:00
//

#include "SP_Bloom.h"
#include "SP_GBuffer.h"
#include "SP_Lighting.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_ShaderManager.h>
#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <ToolDevConsole/TDC_Console.h>

namespace SP
{
    Bloom::Bloom()
    {
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        KGPU::TextureDesc hdrDesc;
        hdrDesc.Width = width;
        hdrDesc.Height = height;
        hdrDesc.MipLevels = BLOOM_MIP_CHAIN;
        hdrDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        hdrDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage | KGPU::TextureUsage::kRenderTarget;

        Gfx::ResourceManager::CreateTexture(BLOOM_TEXTURE_ID, hdrDesc);
        Gfx::ResourceManager::CreateSampler(BLOOM_LINEAR_BORDER_SAMPLER_ID, KGPU::SamplerDesc(KGPU::SamplerAddress::kBorder, KGPU::SamplerFilter::kLinear, false));
        Gfx::ResourceManager::CreateSampler(BLOOM_LINEAR_CLAMP_SAMPLER_ID, KGPU::SamplerDesc(KGPU::SamplerAddress::kClamp, KGPU::SamplerFilter::kLinear, false));
        Gfx::ResourceManager::CreateSampler(BLOOM_POINT_CLAMP_SAMPLER_ID, KGPU::SamplerDesc(KGPU::SamplerAddress::kClamp, KGPU::SamplerFilter::kNearest, false));

        TDC::Console::AddVariable("Graphics.Bloom.Enable", mEnable);
        TDC::Console::AddVariable("Graphics.Bloom.Threshold", mThreshold);
        TDC::Console::AddVariable("Graphics.Bloom.Knee", mKnee);
        TDC::Console::AddVariable("Graphics.Bloom.Strength", mStrength);
        TDC::Console::AddVariable("Graphics.Bloom.Radius", mFilterRadius);

        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/post_fx/bloom/populate_mask.kds");
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/post_fx/bloom/downsample.kds");
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/post_fx/bloom/upsample.kds");
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/post_fx/bloom/composite.kds");
    }

    Bloom::~Bloom()
    {

    }

    void Bloom::Render(RenderPassBegin& begin)
    {
        if (!mEnable)
            return;

        KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render");
    
        CODE_BLOCK("Populate Mask") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(PopulateMask)");

            Gfx::Resource& emissiveInput = Gfx::ResourceManager::Import(GBUFFER_EMISSIVE_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& lightingInput = Gfx::ResourceManager::Import(LIGHTING_OUTPUT_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& bloomOut = Gfx::ResourceManager::Import(BLOOM_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        
            struct PushConstants {
                KGPU::BindlessHandle EmissiveInput;
                KGPU::BindlessHandle LightingInput;
                KGPU::BindlessHandle BloomOut;
                uint Pad;

                int Width;
                int Height;
                KGPU::uint2 Pad2;
            } constants = {
                Gfx::ViewRecycler::GetSRV(emissiveInput.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(lightingInput.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetUAV(bloomOut.Texture)->GetBindlessHandle(),
                0,

                begin.Width,
                begin.Height,
                {}
            };

            auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/populate_mask.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);

            KGPU::TextureBarrier stamp(bloomOut.Texture);
            stamp.BaseMipLevel = 0;
            stamp.LevelCount = 1;
            stamp.SourceAccess = KGPU::ResourceAccess::kShaderWrite;
            stamp.SourceStage = KGPU::PipelineStage::kComputeShader;
            stamp.SourceLayout = KGPU::ResourceLayout::kGeneral;
            stamp.DestAccess = KGPU::ResourceAccess::kShaderWrite;
            stamp.DestStage = KGPU::PipelineStage::kComputeShader;
            stamp.NewLayout = KGPU::ResourceLayout::kGeneral;
            begin.CmdList->Barrier(stamp);

            // Track last access/stage for the *resource*
            bloomOut.LastAccess = KGPU::ResourceAccess::kShaderWrite;
            bloomOut.LastStage = KGPU::PipelineStage::kComputeShader;
            bloomOut.Texture->SetLayout(KGPU::ResourceLayout::kGeneral, 0);
        }

        CODE_BLOCK("Downsample") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(Downsample)");

            Gfx::Resource& bloomMask = Gfx::ResourceManager::Get(BLOOM_TEXTURE_ID);
            Gfx::Resource& linearClamp = Gfx::ResourceManager::Get(BLOOM_LINEAR_CLAMP_SAMPLER_ID);
            for (int i = 0; i < BLOOM_MIP_CHAIN - 1; i++) {
                int width = (int)(bloomMask.Texture->GetDesc().Width * std::pow(0.5f, i));
                int height = (int)(bloomMask.Texture->GetDesc().Height * std::pow(0.5f, i));
            
                KGPU::TextureBarrier mipNBarrier(bloomMask.Texture);
                mipNBarrier.BaseMipLevel = i;
                mipNBarrier.LevelCount = 1;
                mipNBarrier.SourceAccess = KGPU::ResourceAccess::kShaderWrite;         // previously UAV
                mipNBarrier.SourceStage = KGPU::PipelineStage::kComputeShader;        // produced by compute
                mipNBarrier.DestAccess = KGPU::ResourceAccess::kShaderRead;          // now SRV
                mipNBarrier.DestStage = KGPU::PipelineStage::kComputeShader;
                mipNBarrier.SourceLayout = KGPU::ResourceLayout::kGeneral;
                mipNBarrier.NewLayout = KGPU::ResourceLayout::kReadOnly;

                KGPU::TextureBarrier mipNPlusOneBarrier(bloomMask.Texture);
                mipNPlusOneBarrier.BaseMipLevel = i + 1;
                mipNPlusOneBarrier.LevelCount = 1;
                mipNPlusOneBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
                mipNPlusOneBarrier.SourceStage = KGPU::PipelineStage::kAllGraphics;
                mipNPlusOneBarrier.DestAccess = KGPU::ResourceAccess::kShaderWrite;
                mipNPlusOneBarrier.DestStage = KGPU::PipelineStage::kComputeShader;
                mipNPlusOneBarrier.NewLayout = KGPU::ResourceLayout::kGeneral;

                KGPU::TextureViewDesc mipNDesc(bloomMask.Texture, KGPU::TextureViewType::kShaderRead);
                mipNDesc.ViewMip = i;

                KGPU::TextureViewDesc mipNPlusOneDesc(bloomMask.Texture, KGPU::TextureViewType::kShaderWrite);
                mipNPlusOneDesc.ViewMip = i + 1;

                struct PushConstants {
                    KGPU::BindlessHandle MipN;
                    KGPU::BindlessHandle Sampler;
                    KGPU::BindlessHandle MipNPlusOne;
                    uint Pad;

                    int Width;
                    int Height;
                    KGPU::uint2 Pad2;
                } constants = {
                    Gfx::ViewRecycler::GetTextureView(mipNDesc)->GetBindlessHandle(),
                    linearClamp.Sampler->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(mipNPlusOneDesc)->GetBindlessHandle(),
                    0,

                    width,
                    height,
                    {}
                };

                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/downsample.kds");
                begin.CmdList->Barrier({ KC::Array<TextureBarrier>{ mipNBarrier, mipNPlusOneBarrier }, {}, {} });
                begin.CmdList->SetComputePipeline(pipeline);
                begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
                begin.CmdList->Dispatch(std::max((uint)width / 8u, 1u), std::max((uint)height / 8u, 1u), 1);
            }
        }

        CODE_BLOCK("Upsample") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(Upsample)");

            Gfx::Resource& bloomMask = Gfx::ResourceManager::Get(BLOOM_TEXTURE_ID);
            Gfx::Resource& linearClamp = Gfx::ResourceManager::Get(BLOOM_LINEAR_CLAMP_SAMPLER_ID);
            for (int i = BLOOM_MIP_CHAIN - 1; i > 0; i--) {
                int width = (int)(bloomMask.Texture->GetDesc().Width * std::pow(0.5f, i - 1));
                int height = (int)(bloomMask.Texture->GetDesc().Height * std::pow(0.5f, i - 1));
            
                KGPU::TextureBarrier mipNRead(bloomMask.Texture);
                mipNRead.BaseMipLevel = i;
                mipNRead.LevelCount = 1;
                mipNRead.SourceAccess = KGPU::ResourceAccess::kShaderWrite;
                mipNRead.SourceStage = KGPU::PipelineStage::kComputeShader;
                mipNRead.DestAccess = KGPU::ResourceAccess::kShaderRead;
                mipNRead.DestStage = KGPU::PipelineStage::kComputeShader;
                mipNRead.SourceLayout = KGPU::ResourceLayout::kGeneral;
                mipNRead.NewLayout = KGPU::ResourceLayout::kReadOnly;

                KGPU::TextureBarrier mipNm1Write(bloomMask.Texture);
                mipNm1Write.BaseMipLevel = i - 1;
                mipNm1Write.LevelCount = 1;
                mipNm1Write.SourceAccess = KGPU::ResourceAccess::kShaderRead;
                mipNm1Write.SourceStage = KGPU::PipelineStage::kComputeShader;
                mipNm1Write.DestAccess = KGPU::ResourceAccess::kShaderWrite;
                mipNm1Write.DestStage = KGPU::PipelineStage::kComputeShader;
                mipNm1Write.SourceLayout = KGPU::ResourceLayout::kReadOnly;
                mipNm1Write.NewLayout = KGPU::ResourceLayout::kGeneral;

                KGPU::TextureViewDesc mipNDesc(bloomMask.Texture, KGPU::TextureViewType::kShaderRead);
                mipNDesc.ViewMip = i;

                KGPU::TextureViewDesc mipNMinusOneDesc(bloomMask.Texture, KGPU::TextureViewType::kShaderWrite);
                mipNMinusOneDesc.ViewMip = i - 1;

                struct PushConstants {
                    float FilterRadius;
                    KGPU::BindlessHandle Sampler;
                    KGPU::BindlessHandle MipN;
                    KGPU::BindlessHandle MipNPlusOne;

                    int Width;
                    int Height;
                    KGPU::uint2 Pad2;
                } constants = {
                    mFilterRadius,
                    linearClamp.Sampler->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(mipNDesc)->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(mipNMinusOneDesc)->GetBindlessHandle(),

                    width,
                    height,
                    {}
                };

                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/upsample.kds");
                begin.CmdList->Barrier({ KC::Array<TextureBarrier>{ mipNRead, mipNm1Write }, {}, {} });
                begin.CmdList->SetComputePipeline(pipeline);
                begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
                begin.CmdList->Dispatch(std::max((uint)width / 4u, 1u), std::max((uint)height / 4u, 1u), 1);
            }

            KGPU::TextureBarrier finishBarrier(bloomMask.Texture);
            finishBarrier.BaseMipLevel = 0;
            finishBarrier.LevelCount = 1;
            finishBarrier.SourceAccess = KGPU::ResourceAccess::kShaderWrite;
            finishBarrier.SourceStage = KGPU::PipelineStage::kComputeShader;
            finishBarrier.DestAccess = KGPU::ResourceAccess::kShaderRead;
            finishBarrier.DestStage = KGPU::PipelineStage::kAllGraphics;
            finishBarrier.NewLayout = KGPU::ResourceLayout::kReadOnly;
            begin.CmdList->Barrier(finishBarrier);

            bloomMask.LastAccess = KGPU::ResourceAccess::kShaderRead;
            bloomMask.LastStage = KGPU::PipelineStage::kAllGraphics;
        }

        CODE_BLOCK("Composite") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(Composite)");

            // Already put in read only :3
            Gfx::Resource& bloomMask = Gfx::ResourceManager::Get(BLOOM_TEXTURE_ID);
            Gfx::Resource& output = Gfx::ResourceManager::Import(LIGHTING_OUTPUT_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
            Gfx::Resource& linearClamp = Gfx::ResourceManager::Get(BLOOM_LINEAR_CLAMP_SAMPLER_ID);

            struct PushConstants {
                KGPU::BindlessHandle Input;
                KGPU::BindlessHandle InputSampler;
                KGPU::BindlessHandle Output;
                float Strength;

                int Width;
                int Height;
                KGPU::uint2 Pad;
            } constants = {
                Gfx::ViewRecycler::GetSRV(bloomMask.Texture)->GetBindlessHandle(),
                linearClamp.Sampler->GetBindlessHandle(),
                Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
                mStrength,

                begin.Width,
                begin.Height,
                {}
            };

            auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/composite.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        }
    }

    void Bloom::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Bloom", ImGuiTreeNodeFlags_Framed)) {
            ImGui::Checkbox("Enable", &mEnable);
            ImGui::SliderFloat("Luminance Threshold", &mThreshold, 1.0f, 2.0f);
            ImGui::SliderFloat("Threshold Knee", &mKnee, 0.5f, 1.0f);
            ImGui::SliderFloat("Strength", &mStrength, 0.1f, 3.0f);
            ImGui::TreePop();
        }
    }
}
