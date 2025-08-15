//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 20:45:01
//

#include "SP_Shadows.h"
#include "SP_GBuffer.h"
#include "SP_Application.h"

#include <KernelCore/KC_Math.h>
#include <Effects/FX_DebugRenderer.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_ShaderManager.h>
#include <ToolDevConsole/TDC_Console.h>

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

namespace SP
{
    Shadows::Shadows()
    {
        TDC::Console::AddVariable("Graphics.Shadow.AlphaTest", mAlphaTest);
        TDC::Console::AddVariable("Graphics.Shadow.UseRTPipeline", mUsePipeline);
        TDC::Console::AddVariable("Graphics.Shadow.CSM.LambdaSplit", mSplitLambda);
        TDC::Console::AddVariable("Graphics.Shadow.CSM.Update", mUpdateCascades);
        TDC::Console::AddVariable("Graphics.Shadow.RT.NormalBias", mNormalBias);
        TDC::Console::AddVariable("Graphics.Shadow.RT.LightRadius", mLightRadius);
        TDC::Console::AddVariable("Graphics.Shadow.RT.Accumulate", mAccumulate);
        TDC::Console::AddVariable("Graphics.Shadow.RT.ATrous", mDoAtrous);
        TDC::Console::AddVariable("Graphics.Shadow.RT.SigmaNormal", mSVGFSigmaNormal);
        TDC::Console::AddVariable("Graphics.Shadow.RT.SigmaDepth", mSVGFSigmaDepth);
        TDC::Console::AddVariable("Graphics.Shadow.RT.SigmaVariance", mSVGFSigmaVariance);

        // Textures
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);
        KGPU::IDevice* device = Gfx::Manager::GetDevice();

        // Create mask
        KGPU::TextureDesc sunMaskDesc;
        sunMaskDesc.Width = width;
        sunMaskDesc.Height = height;
        sunMaskDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kRenderTarget | KGPU::TextureUsage::kStorage;
        
        sunMaskDesc.Format = KGPU::TextureFormat::kR16_FLOAT;
        Gfx::ResourceManager::CreateTexture(SHADOWS_SUN_MASK_ID, sunMaskDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_PREVIOUS_SUN_MASK_ID, sunMaskDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_SUN_MASK_SCRATCH_ID, sunMaskDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_SUN_MASK_SCRATCH2_ID, sunMaskDesc);

        sunMaskDesc.Format = KGPU::TextureFormat::kR16_UINT;
        Gfx::ResourceManager::CreateTexture(SHADOWS_SUN_MASK_LENGTH_ID, sunMaskDesc);
        
        sunMaskDesc.Format = KGPU::TextureFormat::kR16G16_FLOAT;
        Gfx::ResourceManager::CreateTexture(SHADOWS_MOMENTS_ID, sunMaskDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_PREV_MOMENTS_ID, sunMaskDesc);

        // Create cascades and cascade buffer
        KGPU::TextureDesc cascadeDesc;
        cascadeDesc.Width = SHADOW_CASCADE_QUALITY;
        cascadeDesc.Height = cascadeDesc.Width;
        cascadeDesc.Format = KGPU::TextureFormat::kD32_FLOAT;
        cascadeDesc.Usage = KGPU::TextureUsage::kDepthTarget | KGPU::TextureUsage::kShaderResource;

        Gfx::ResourceManager::CreateTexture(SHADOWS_CASCADE_0, cascadeDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_CASCADE_1, cascadeDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_CASCADE_2, cascadeDesc);
        Gfx::ResourceManager::CreateTexture(SHADOWS_CASCADE_3, cascadeDesc);
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            mCascadeBuffers[i] = device->CreateBuffer(KGPU::BufferDesc(sizeof(ShadowCascade) * SHADOW_CASCADE_COUNT, sizeof(ShadowCascade), KGPU::BufferUsage::kStaging |  KGPU::BufferUsage::kShaderRead));
            mCascadeBuffers[i]->SetName("Cascade Buffer");
        }

        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            const char* indexToID[4] = { SHADOWS_CASCADE_0, SHADOWS_CASCADE_1,SHADOWS_CASCADE_2, SHADOWS_CASCADE_3 };
            Gfx::Resource& depthTexture = Gfx::ResourceManager::Get(indexToID[i]);

            mCascades[i].SRVIndex = Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(depthTexture.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle();
        }

        // Create pipelines
        CODE_BLOCK("Create CSM resources") {
            KGPU::GraphicsPipelineDesc pipelineDesc = {};
            pipelineDesc.RenderTargetFormats = {};
            pipelineDesc.DepthEnabled = true;
            pipelineDesc.DepthWrite = true;
            pipelineDesc.DepthClampEnabled = true;
            pipelineDesc.DepthFormat = KGPU::TextureFormat::kD32_FLOAT;
            pipelineDesc.DepthOperation = KGPU::DepthOperation::kLess;
            pipelineDesc.CullMode = KGPU::CullMode::kBack;

            Gfx::ShaderManager::SubscribeGraphics("data/sp/shaders/shadows/csm/alpha.kds", pipelineDesc);
            Gfx::ShaderManager::SubscribeGraphics("data/sp/shaders/shadows/csm/no_alpha.kds", pipelineDesc);
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/csm/populate.kds");
        }
        CODE_BLOCK("Create Hard RT resources") {
            RaytracingPipelineDesc hardRTDesc;
            hardRTDesc.PayloadDesc = sizeof(uint);
            hardRTDesc.PushConstantSize = sizeof(uint) * 12;
            hardRTDesc.RecursionDepth = 1;

            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/hard_rt/alpha.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/hard_rt/no_alpha.kds");
            Gfx::ShaderManager::SubscribeRaytracing("data/sp/shaders/shadows/hard_rt/no_alpha_pipeline.kds", hardRTDesc);
            Gfx::ShaderManager::SubscribeRaytracing("data/sp/shaders/shadows/hard_rt/alpha_pipeline.kds", hardRTDesc);
        }
        CODE_BLOCK("Create Soft RT resources") {
            RaytracingPipelineDesc hardRTDesc;
            hardRTDesc.PayloadDesc = sizeof(uint);
            hardRTDesc.PushConstantSize = sizeof(uint) * 16;
            hardRTDesc.RecursionDepth = 1;

            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/soft_rt/alpha.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/soft_rt/no_alpha.kds");
            Gfx::ShaderManager::SubscribeRaytracing("data/sp/shaders/shadows/soft_rt/no_alpha_pipeline.kds", hardRTDesc);
            Gfx::ShaderManager::SubscribeRaytracing("data/sp/shaders/shadows/soft_rt/alpha_pipeline.kds", hardRTDesc);
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/denoise/svgf_temporal.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/denoise/svgf_spatial.kds");
            Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/shadows/denoise/ground_truth.kds");
        }
    }

    Shadows::~Shadows()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            KC_DELETE(mCascadeBuffers[i]);
        }
    }

    void Shadows::Render(RenderPassBegin& begin)
    {
        switch (mMode) {
            case ShadowMode::kNone: {
                None(begin);
                break;
            }
            case ShadowMode::kCSM: {
                CSM(begin);
                break;
            }
            case ShadowMode::kHardRT: {
                if (Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                    HardRT(begin);
                } else {
                    None(begin);
                }
                break;
            }
            case ShadowMode::kSoftRT: {
                if (Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                    SoftRT(begin);
                } else {
                    None(begin);
                }
                break;
            }
            default: {
                None(begin);
                break;
            }
        }
    }

    void Shadows::None(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::None");
        Gfx::Resource& before = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kColorWrite);

        KGPU::RenderAttachment attachment(Gfx::ViewRecycler::GetRTV(before.Texture), true, float3(1.0f));
        KGPU::RenderBegin renderBegin(before.Texture->GetDesc().Width, before.Texture->GetDesc().Height, { attachment }, {});

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->EndRendering();
    }

    void Shadows::CSM(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::CSM");

        UpdateCascades(begin);
        DrawCascades(begin);
        PopulateCSMVisibilityMask(begin);
    }

    void Shadows::UpdateCascades(RenderPassBegin& begin)
    {
        CODE_BLOCK("Update Cascade Matrices") {
            uint cascadeSize = SHADOW_CASCADE_QUALITY;
            KC::Array<float> splits(SHADOW_CASCADE_COUNT + 1);

            splits[0] = CAMERA_NEAR;
            splits[SHADOW_CASCADE_COUNT] = CAMERA_FAR;
            for (int i = 1; i <= SHADOW_CASCADE_COUNT; ++i) {
                float fraction = static_cast<float>(i) / SHADOW_CASCADE_COUNT;
                float linearSplit = CAMERA_NEAR + (CAMERA_FAR - CAMERA_NEAR) * fraction;
                float logSplit = CAMERA_NEAR * std::pow(CAMERA_FAR / CAMERA_NEAR, fraction);
                splits[i] = mSplitLambda * logSplit + (1.0f - mSplitLambda) * linearSplit;
            }

            for (int i = 0; i < SHADOW_CASCADE_COUNT; ++i) {
                glm::mat4 projection = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, splits[i], splits[i + 1]);
                KC::Array<float4> corners = KC::Math::GetFrustumCorners(begin.CamData.View, projection);

                // Calculate center
                float3 center(0.0f);
                for (const float4& corner : corners) {
                    center += float3(corner);
                }
                center /= corners.size();

                // Adjust light's up vector
                float3 up(0.0f, 1.0f, 0.0f);
                if (glm::abs(glm::dot(begin.World->GetLightList()->Sun.Direction, up)) > 0.999f) {
                    up = float3(1.0f, 0.0f, 0.0f);
                }

                // Calculate light-space bounding sphere
                float3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
                float sphereRadius = 0.0f;
                for (auto& corner : corners) {
                    float dist = glm::length(float3(corner) - center);
                    sphereRadius = std::max(sphereRadius, dist);
                }
                sphereRadius = (std::ceil(sphereRadius * 16.0f) / 16.0f);
                maxBounds = float3(sphereRadius);
                minBounds = -maxBounds;

                // Get extents and create view matrix
                float3 cascadeExtents = maxBounds - minBounds;
                float3 shadowCameraPos = center - begin.World->GetLightList()->Sun.Direction;

                glm::mat4 lightView = glm::lookAt(shadowCameraPos, center, up);
                glm::mat4 lightProjection = glm::ortho(
                    minBounds.x,
                    maxBounds.x,
                    minBounds.y,
                    maxBounds.y,
                    minBounds.z,
                    maxBounds.z
                );

                // Texel snap
                {
                    glm::mat4 shadowMatrix = lightProjection * lightView;
                    glm::vec4 shadowOrigin = float4(0.0f, 0.0f, 0.0f, 1.0f);
                    shadowOrigin = shadowMatrix * shadowOrigin;
                    shadowOrigin = glm::scale(glm::mat4(1.0f), float3(cascadeSize / 2)) * shadowOrigin;
                
                    float4 roundedOrigin = glm::round(shadowOrigin);
                    float4 roundOffset = roundedOrigin - shadowOrigin;
                    roundOffset = roundOffset * (2.0f / cascadeSize);
                    roundOffset.z = 0.0f;
                    roundOffset.w = 0.0f;
                    lightProjection[3] += roundOffset;
                }

                // Store results or draw
                if (!mUpdateCascades) {
                    FX::DebugRenderer::DrawFrustum(mCascades[i].Proj * mCascades[i].View);
                } else {
                    mCascades[i].Split = splits[i + 1];
                    mCascades[i].View = lightView;
                    mCascades[i].Proj = lightProjection;
                }
            }

            void* ptr = mCascadeBuffers[begin.FrameIndex]->Map();
            memcpy(ptr, mCascades.data(), mCascades.size() * sizeof(ShadowCascade));
            mCascadeBuffers[begin.FrameIndex]->Unmap();
        }
    }

    void Shadows::DrawCascades(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::DrawCascades");

        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            KGPU::ScopedMarker marker(begin.CmdList, "Cascade " + std::to_string(i));

            KGPU::IGraphicsPipeline* pipeline = mAlphaTest
                                           ? Gfx::ShaderManager::GetGraphics("data/sp/shaders/shadows/csm/alpha.kds")
                                           : Gfx::ShaderManager::GetGraphics("data/sp/shaders/shadows/csm/no_alpha.kds");
            const char* indexToID[4] = { SHADOWS_CASCADE_0, SHADOWS_CASCADE_1,SHADOWS_CASCADE_2, SHADOWS_CASCADE_3 };

            Gfx::Resource& depthTexture = Gfx::ResourceManager::Import(indexToID[i], begin.CmdList, Gfx::ImportType::kDepthWrite);
            Gfx::Resource& defaultWhite = Gfx::ResourceManager::Get(Gfx::DEFAULT_WHITE_TEXTURE);
            Gfx::Resource& materialSampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
            KGPU::RenderBegin renderBegin(SHADOW_CASCADE_QUALITY, SHADOW_CASCADE_QUALITY, {}, KGPU::RenderAttachment(Gfx::ViewRecycler::GetDSV(depthTexture.Texture)));

            begin.CmdList->BeginRendering(renderBegin);
            begin.CmdList->SetGraphicsPipeline(pipeline);
            begin.CmdList->SetRenderSize(SHADOW_CASCADE_QUALITY, SHADOW_CASCADE_QUALITY);
            begin.World->ForEach([&](RenderEntity entity, Gfx::Material* material) {
                KGPU::BindlessHandle albedoView = material->GetAlbedoView() ? material->GetAlbedoView()->GetBindlessHandle() : Gfx::ViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle();

                struct PushConstants {
                    BindlessHandle VertexBuffer;
                    BindlessHandle Albedo;
                    BindlessHandle Sampler;
                    uint Pad;

                    glm::mat4 View;
                    glm::mat4 Proj;
                } constant = {
                    entity.Primitive->GetVertexBufferView()->GetBindlessHandle(),
                    albedoView,
                    materialSampler.Sampler->GetBindlessHandle(),
                    0,

                    mCascades[i].View,
                    mCascades[i].Proj
                };

                begin.CmdList->SetIndexBuffer(entity.Primitive->GetIndexBuffer());
                begin.CmdList->SetGraphicsConstants(pipeline, &constant, sizeof(constant));
                begin.CmdList->DrawIndexed(entity.Primitive->GetIndexCount(), 1, 0, 0, 0);
            });
            begin.CmdList->EndRendering();
        }
    }

    void Shadows::PopulateCSMVisibilityMask(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::PopulateCSMVisibilityMask");

        const char* indexToID[4] = { SHADOWS_CASCADE_0, SHADOWS_CASCADE_1,SHADOWS_CASCADE_2, SHADOWS_CASCADE_3 };
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            (void)Gfx::ResourceManager::Import(indexToID[i], begin.CmdList, Gfx::ImportType::kShaderRead);
        }

        Gfx::Resource& output = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& gbufferDepth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferNormal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        Gfx::Resource& materialSampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);

        struct PushConstants {
            BindlessHandle CascadeIndex;
            BindlessHandle SamplerIndex;
            BindlessHandle CameraIndex;
            BindlessHandle DepthIndex;

            BindlessHandle OutputIndex;
            BindlessHandle SunIndex;
            BindlessHandle NormalIndex;
            uint Pad;

            int Width;
            int Height;
            uint2 Pad1;
        } constants = {
            Gfx::ViewRecycler::GetSRV(mCascadeBuffers[begin.FrameIndex])->GetBindlessHandle(),
            materialSampler.Sampler->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(gbufferDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),

            Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            begin.World->GetLightList()->GetSunBufferView(begin.FrameIndex)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),
            0,

            begin.Width,
            begin.Height,
            uint2(0)
        };

        KGPU::IComputePipeline* pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/csm/populate.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
    }

    void Shadows::HardRT(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::HardRT");

        Gfx::Resource& output = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& gbufferDepth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferNormal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        Gfx::Resource& materialSampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);

        struct PushConstants {
            BindlessHandle SunArray;
            BindlessHandle Output;
            BindlessHandle AS;
            float NormalBias;

            int Width;
            int Height;
            BindlessHandle Depth;
            BindlessHandle Normal;

            BindlessHandle Camera;
            BindlessHandle Sampler;
            BindlessHandle Instances;
            BindlessHandle Materials;
        } constants = {
            begin.World->GetLightList()->GetSunBufferView(begin.FrameIndex)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            begin.World->GetRTWorld()->GetWorldView(),
            mNormalBias,

            begin.Width,
            begin.Height,
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(gbufferDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),

            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            materialSampler.Sampler->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(begin.World->GetSceneInstanceBuffer())->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(begin.World->GetSceneMaterialBuffer())->GetBindlessHandle()
        };

        if (!mUsePipeline) {
            KGPU::IComputePipeline* pipeline = mAlphaTest
                                          ? Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/hard_rt/alpha.kds")
                                          : Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/hard_rt/no_alpha.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        } else {
            KGPU::IRaytracingPipeline* pipeline = mAlphaTest
                                          ? Gfx::ShaderManager::GetRaytracing("data/sp/shaders/shadows/hard_rt/alpha_pipeline.kds")
                                          : Gfx::ShaderManager::GetRaytracing("data/sp/shaders/shadows/hard_rt/no_alpha_pipeline.kds");
            begin.CmdList->SetRaytracingPipeline(pipeline);
            begin.CmdList->SetRaytracingConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->DispatchRays(pipeline, begin.Width, begin.Height, 1);
        }
    }

    void Shadows::SoftRT(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::SoftRT");

        TraceSoftShadowRays(begin);
        switch (mDenoiser) {
            case ShadowDenoiser::kGroundTruth: {
                DenoiseGroundTruth(begin);
                break;
            }
            case ShadowDenoiser::kSVGF: {
                SVGFTemporal(begin);
                SVGFSpatial(begin);
                CopyHistory(begin);
                break;
            }
        }
    }
    
    void Shadows::CopyHistory(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::CopyHistory");

        Gfx::Resource& output = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kTransferSource);
        Gfx::Resource& prev = Gfx::ResourceManager::Import(SHADOWS_PREVIOUS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kTransferDest);
        Gfx::Resource& moments = Gfx::ResourceManager::Import(SHADOWS_MOMENTS_ID, begin.CmdList, Gfx::ImportType::kTransferSource);
        Gfx::Resource& prevMoments = Gfx::ResourceManager::Import(SHADOWS_PREV_MOMENTS_ID, begin.CmdList, Gfx::ImportType::kTransferDest);
    
        begin.CmdList->CopyTextureToTexture(prev.Texture, output.Texture);
        begin.CmdList->CopyTextureToTexture(prevMoments.Texture, moments.Texture);
    }
    
    void Shadows::TraceSoftShadowRays(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::TraceSoftShadowRays");

        Gfx::Resource& output = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& gbufferDepth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& gbufferNormal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        Gfx::Resource& materialSampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);

        struct PushConstants {
            BindlessHandle SunArray;
            BindlessHandle Output;
            BindlessHandle AS;
            float NormalBias;

            int Width;
            int Height;
            BindlessHandle Depth;
            BindlessHandle Normal;

            BindlessHandle Camera;
            BindlessHandle Sampler;
            BindlessHandle Instances;
            uint FrameIndex;

            float LightRadius;
            BindlessHandle Materials;
            KGPU::float2 Pad;
        } constants = {
            begin.World->GetLightList()->GetSunBufferView(begin.FrameIndex)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            begin.World->GetRTWorld()->GetWorldView(),
            mNormalBias,

            begin.Width,
            begin.Height,
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(gbufferDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),

            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            materialSampler.Sampler->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(begin.World->GetSceneInstanceBuffer())->GetBindlessHandle(),
            begin.FrameCount,

            mLightRadius,
            Gfx::ViewRecycler::GetSRV(begin.World->GetSceneMaterialBuffer())->GetBindlessHandle(),
            {}
        };

        if (!mUsePipeline) {
            KGPU::IComputePipeline* pipeline = mAlphaTest
                                          ? Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/soft_rt/alpha.kds")
                                          : Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/soft_rt/no_alpha.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        } else {
            KGPU::IRaytracingPipeline* pipeline = mAlphaTest
                                          ? Gfx::ShaderManager::GetRaytracing("data/sp/shaders/shadows/soft_rt/alpha_pipeline.kds")
                                          : Gfx::ShaderManager::GetRaytracing("data/sp/shaders/shadows/soft_rt/no_alpha_pipeline.kds");
            begin.CmdList->SetRaytracingPipeline(pipeline);
            begin.CmdList->SetRaytracingConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->DispatchRays(pipeline, begin.Width, begin.Height, 1);
        }
    }

    void Shadows::DenoiseGroundTruth(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::DenoiseGroundTruth");
        CODE_BLOCK("Denoise") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::DenoiseGroundTruth(Denoise)");

            // Import resources
            Gfx::Resource& prevMask = Gfx::ResourceManager::Import(SHADOWS_PREVIOUS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& currentMask = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
            Gfx::Resource& historyID = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_LENGTH_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
            Gfx::Resource& motionVec = Gfx::ResourceManager::Import(GBUFFER_MOTION_VECTOR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& nearestSampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);

            struct PushConstants {
                KGPU::BindlessHandle PrevID;
                KGPU::BindlessHandle CurrID;
                KGPU::BindlessHandle HistoryID;
                KGPU::BindlessHandle MotionID;
            
                int Width;
                int Height;
                KGPU::BindlessHandle SamplerID;
                uint Pad;
            } constants = {
                Gfx::ViewRecycler::GetSRV(prevMask.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetUAV(currentMask.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(historyID.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(motionVec.Texture)->GetBindlessHandle(),

                begin.Width,
                begin.Height,
                nearestSampler.Sampler->GetBindlessHandle(),
                0
            };

            auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/denoise/ground_truth.kds");
            begin.CmdList->SetComputePipeline(pipeline);
            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        }

        CODE_BLOCK("Copy to History") {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::DenoiseGroundTruth(Copy)");

            Gfx::Resource& currentMask   = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kTransferSource);
            Gfx::Resource& prevMask  = Gfx::ResourceManager::Import(SHADOWS_PREVIOUS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kTransferDest);

            begin.CmdList->CopyTextureToTexture(prevMask.Texture, currentMask.Texture);
        }
    }

    void Shadows::SVGFTemporal(RenderPassBegin& begin)
    {
        if (!mAccumulate)
            return;

        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::SVGFTemporal");
    
        Gfx::Resource& prevMask = Gfx::ResourceManager::Import(SHADOWS_PREVIOUS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& currentMask = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& motionVector = Gfx::ResourceManager::Import(GBUFFER_MOTION_VECTOR_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& normal = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& prevNormal = Gfx::ResourceManager::Import(GBUFFER_PREV_NORMAL_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& depth = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& prevDepth = Gfx::ResourceManager::Import(GBUFFER_PREV_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& moments = Gfx::ResourceManager::Import(SHADOWS_MOMENTS_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& prevMoments = Gfx::ResourceManager::Import(SHADOWS_PREV_MOMENTS_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
        Gfx::Resource& history = Gfx::ResourceManager::Import(SHADOWS_SUN_MASK_LENGTH_ID, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);

        struct Constants {
            KGPU::BindlessHandle PrevID;
            KGPU::BindlessHandle CurrID;
            KGPU::BindlessHandle HistoryID;
            KGPU::BindlessHandle MotionVectorID;

            int Width;
            int Height;
            KGPU::BindlessHandle NormalID;
            KGPU::BindlessHandle PrevNormalID;

            KGPU::BindlessHandle DepthID;
            KGPU::BindlessHandle PrevDepthID;
            KGPU::BindlessHandle CameraBuffer;
            KGPU::BindlessHandle BilinearSamplerID;

            KGPU::BindlessHandle PrevMoments;
            KGPU::BindlessHandle CurrMoments;
            KGPU::float2 Pad;
        } constants = {
            Gfx::ViewRecycler::GetSRV(prevMask.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(currentMask.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(history.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(motionVector.Texture)->GetBindlessHandle(),

            begin.Width,
            begin.Height,
            Gfx::ViewRecycler::GetSRV(normal.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(prevNormal.Texture)->GetBindlessHandle(),

            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(depth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(prevDepth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            sampler.Sampler->GetBindlessHandle(),

            Gfx::ViewRecycler::GetUAV(moments.Texture)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetSRV(prevMoments.Texture)->GetBindlessHandle(),
            {}
        };

        auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/denoise/svgf_temporal.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
    }

    void Shadows::SVGFSpatial(RenderPassBegin& begin)
    {
        if (!mDoAtrous)
            return;

        KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::SVGFSpatial");

        auto pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/shadows/denoise/svgf_spatial.kds");
        begin.CmdList->SetComputePipeline(pipeline);

        const int atrousIterations = 3;
        for (int i = 0; i < atrousIterations; i++) {
            KGPU::ScopedMarker _(begin.CmdList, "SP::Shadows::SVGFSpatial(Iteration=" + std::to_string(i) + ")");

            const char* srcID = (i % 2 == 0) ? SHADOWS_SUN_MASK_ID : SHADOWS_SUN_MASK_SCRATCH_ID;
            const char* dstID = (i % 2 == 0) ? SHADOWS_SUN_MASK_SCRATCH_ID : SHADOWS_SUN_MASK_ID;

            Gfx::Resource& src = Gfx::ResourceManager::Import(srcID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& dst = Gfx::ResourceManager::Import(dstID, begin.CmdList, Gfx::ImportType::kShaderWrite);
            Gfx::Resource& normal = Gfx::ResourceManager::Get(GBUFFER_NORMAL_ID);
            Gfx::Resource& depth = Gfx::ResourceManager::Get(GBUFFER_DEPTH_ID);
            Gfx::Resource& moments = Gfx::ResourceManager::Import(SHADOWS_MOMENTS_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
            Gfx::Resource& sampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID);

            struct Constants {
                KGPU::BindlessHandle SrcID;
                KGPU::BindlessHandle DstID;
                KGPU::BindlessHandle NormalID;
                KGPU::BindlessHandle DepthID;
            
                KGPU::BindlessHandle MomentsID;
                int Width;
                int Height;
                float SigmaNormal;
            
                float SigmaDepth;
                float SigmaVariance;
                uint Stride;
                KGPU::BindlessHandle Sampler;
            } constants = {
                Gfx::ViewRecycler::GetSRV(src.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetUAV(dst.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetSRV(normal.Texture)->GetBindlessHandle(),
                Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(depth.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),

                Gfx::ViewRecycler::GetSRV(moments.Texture)->GetBindlessHandle(),
                begin.Width,
                begin.Height,
                mSVGFSigmaNormal,

                mSVGFSigmaDepth,
                mSVGFSigmaVariance,
                1u << i,
                sampler.Sampler->GetBindlessHandle()
            };

            begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->Dispatch((begin.Width + 7) / 8, (begin.Height + 7) / 8, 1);
        }
    }

    void Shadows::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("Shadows", ImGuiTreeNodeFlags_Framed)) {
        // Shadow modes
        const char* modes[] = { "None", "CSM", "Hard RT", "Soft RT" };
            
        if (ImGui::BeginCombo("Shadow Technique", modes[(int)mMode])) {
            for (int i = 0; i < IM_ARRAYSIZE(modes); i++) {
                bool disabled = false;
            
                // Disable RT modes if device doesn’t support raytracing
                if ((i == (int)ShadowMode::kHardRT || i == (int)ShadowMode::kSoftRT) 
                    && !Gfx::Manager::GetDevice()->SupportsRaytracing()) {
                    disabled = true;
                }
            
                if (disabled) {
                    ImGui::BeginDisabled();
                    ImGui::Selectable(modes[i], false);
                    ImGui::EndDisabled();
                } else {
                    bool isSelected = (mMode == (ShadowMode)i);
                    if (ImGui::Selectable(modes[i], isSelected)) {
                        mMode = (ShadowMode)i;
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    
        ImGui::Checkbox("Alpha Test", &mAlphaTest);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
            ImGui::SetTooltip("Whether or not to use alpha testing for shadows. This can tank performance depending on the technique, so use at your own risk.");
        }

        if (Gfx::Manager::GetDevice()->SupportsRaytracing()) {
            ImGui::Checkbox("RT Pipeline instead of RayQuery", &mUsePipeline);
        } else {
            ImGui::BeginDisabled();
            ImGui::Checkbox("RT Pipeline instead of RayQuery", &mUsePipeline);
            ImGui::EndDisabled();
        }
    
        switch (mMode) {
            case ShadowMode::kSoftRT: {
                ImGui::SliderFloat("Normal Bias", &mNormalBias, 0.001f, 0.01f);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("The offset at which the shadow ray is traced based on the surface normal.");
                }
            
                ImGui::SliderFloat("Light Radius", &mLightRadius, 0.0f, 10.0f);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("When tracing shadow rays, the directional light is treated as an area light to generate random shadow rays sampled on its surface. You can pick the area light radius using this slider.");
                }

                const char* denoiserModes[] = { "Ground Truth", "SVGF" };
                ImGui::Combo("Denoiser", (int*)&mDenoiser, denoiserModes, IM_ARRAYSIZE(denoiserModes));
                if (ImGui::TreeNodeEx("Denoiser Settings", ImGuiTreeNodeFlags_Framed)) {
                    if (mDenoiser == ShadowDenoiser::kSVGF) {
                        ImGui::Checkbox("Accumulate", &mAccumulate);
                        ImGui::Checkbox("Do A-Trous", &mDoAtrous);
                        ImGui::SliderFloat("Sigma Variance", &mSVGFSigmaVariance, 0.1f, 16.0f);
                        ImGui::SliderFloat("Sigma Normal", &mSVGFSigmaNormal, 0.1f, 256.0f);
                        ImGui::SliderFloat("Sigma Depth", &mSVGFSigmaDepth, 0.1f, 2.0f);
                    }
                    ImGui::TreePop();
                }
                break;
            }
            case ShadowMode::kHardRT: {
                ImGui::SliderFloat("Normal Bias", &mNormalBias, 0.001f, 0.01f);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("The offset at which the shadow ray is traced based on the surface normal.");
                }
                break;
            }
            case ShadowMode::kCSM: {
                ImGui::SliderFloat("Shadow Split Lambda", &mSplitLambda, 0.01f, 0.99f);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("Used to calculate the repartition of cascade splits through the view depth. Lower values mean a more linear repartition whilst higher values means a more exponential repartition.");
                }
            
                ImGui::Checkbox("Update Cascades", &mUpdateCascades);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
                    ImGui::SetTooltip("When checked off, cascade matrices will not be updated and will be drawn using the debug renderer.");
                }
                break;
            }
            default:
                break;
        }
        ImGui::TreePop();
    }
    }
}
