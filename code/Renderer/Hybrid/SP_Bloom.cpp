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
        TDC::Console::AddVariable("Graphics.Bloom.UpsampleGain", mUpsampleGain);
        TDC::Console::AddVariable("Graphics.Bloom.OnlyEmissive", mOnlyEmissive);

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
                float Threshold;

                int Width;
                int Height;
                float ThresholdKnee;
                uint OnlyEmissive;
            } constants = {
                Gfx::ViewRecycler::GetSRV(emissiveInput.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(lightingInput.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetUAV(bloomOut.Texture)->GetBindlessHandle(),
                mThreshold,

                begin.Width,
                begin.Height,
                mKnee,
                mOnlyEmissive
            };

            auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/populate_mask.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        }

        CODE_BLOCK("Downsample") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(Downsample)");

            Gfx::Resource& linearClamp = Gfx::ResourceManager::Get(BLOOM_LINEAR_CLAMP_SAMPLER_ID);
            for (int i = 0; i < BLOOM_MIP_CHAIN - 1; i++) {
                Gfx::Resource& bloomMaskMipN = Gfx::ResourceManager::Import(BLOOM_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderRead, i);
                Gfx::Resource& bloomMaskMipNPlusOne = Gfx::ResourceManager::Import(BLOOM_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderWrite, i + 1);

                KGPU::TextureViewDesc mipNDesc(bloomMaskMipN.Texture, KGPU::TextureViewType::kShaderRead);
                mipNDesc.ViewMip = i;
                
                KGPU::TextureViewDesc mipNPlusOneDesc(bloomMaskMipN.Texture, KGPU::TextureViewType::kShaderWrite);
                mipNPlusOneDesc.ViewMip = i + 1;

                struct PushConstants {
                    KGPU::BindlessHandle MipN;
                    KGPU::BindlessHandle Sampler;
                    KGPU::BindlessHandle MipNPlusOne;
                    uint Pad;
                } constants = {
                    Gfx::ViewRecycler::GetTextureView(mipNDesc)->GetBindlessHandle(),
                    linearClamp.Sampler->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(mipNPlusOneDesc)->GetBindlessHandle(),
                    0,
                };

                const int srcW = std::max(1, int(bloomMaskMipN.Texture->GetDesc().Width  >> i));
                const int srcH = std::max(1, int(bloomMaskMipN.Texture->GetDesc().Height >> i));
                const int dstW = std::max(1, srcW >> 1);
                const int dstH = std::max(1, srcH >> 1);

                const uint gx = (dstW + 7) / 8;
                const uint gy = (dstH + 7) / 8;
            
                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/downsample.kds");
                begin.CmdList->SetComputePipeline(pipeline);
                begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
                begin.CmdList->Dispatch(std::max(gx, 1u), std::max(gy, 1u), 1);
            }
        }

        CODE_BLOCK("Upsample") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(Upsample)");

            Gfx::Resource& bloomMask = Gfx::ResourceManager::Get(BLOOM_TEXTURE_ID);
            Gfx::Resource& linearClamp = Gfx::ResourceManager::Get(BLOOM_LINEAR_CLAMP_SAMPLER_ID);
            for (int i = BLOOM_MIP_CHAIN - 1; i > 0; --i) {
                Gfx::Resource& bloomMaskMipN = Gfx::ResourceManager::Import(BLOOM_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderRead, i);
                Gfx::Resource& bloomMaskMipNPlusOne = Gfx::ResourceManager::Import(BLOOM_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderWrite, i - 1);

                const int dstW = std::max(1, int(bloomMask.Texture->GetDesc().Width  >> (i - 1)));
                const int dstH = std::max(1, int(bloomMask.Texture->GetDesc().Height >> (i - 1)));

                KGPU::TextureViewDesc mipNDesc(bloomMask.Texture, KGPU::TextureViewType::kShaderRead);
                mipNDesc.ViewMip = i;

                KGPU::TextureViewDesc mipNMinusOneDesc(bloomMask.Texture, KGPU::TextureViewType::kShaderWrite);
                mipNMinusOneDesc.ViewMip = i - 1;

                struct Push {
                    float FilterRadius;
                    KGPU::BindlessHandle Sampler;
                    KGPU::BindlessHandle MipN;
                    KGPU::BindlessHandle MipNMinusOne;
                } pc = {
                    mUpsampleGain,
                    linearClamp.Sampler->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(mipNDesc)->GetBindlessHandle(),
                    Gfx::ViewRecycler::GetTextureView(mipNMinusOneDesc)->GetBindlessHandle(),
                };
            
                const uint gx = (dstW + 7) / 8;
                const uint gy = (dstH + 7) / 8;
            
                auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/post_fx/bloom/upsample.kds");
                begin.CmdList->SetComputePipeline(pipeline);
                begin.CmdList->SetComputeConstants(pipeline, &pc, sizeof(pc));
                begin.CmdList->Dispatch(std::max(gx, 1u), std::max(gy, 1u), 1);
            }
        }

        CODE_BLOCK("Composite") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Bloom::Render(Composite)");

            // First transition from write to read
            Gfx::ResourceManager::Import(BLOOM_TEXTURE_ID, begin.CmdList, Gfx::ImportType::kShaderRead, 0);
            
            // Then use it as read-only
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
            ImGui::Checkbox("Only Use Emissive", &mOnlyEmissive);
            ImGui::SliderFloat("Luminance Threshold", &mThreshold, 0.1f, 20.0f);
            ImGui::SliderFloat("Threshold Knee", &mKnee, 0.1f, 20.0f);
            ImGui::SliderFloat("Strength", &mStrength, 0.1f, 5.0f);
            ImGui::SliderFloat("Upsample Gain", &mUpsampleGain, 0.1f, 2.0f);
            ImGui::TreePop();
        }
    }
}
