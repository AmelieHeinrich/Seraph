//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:29:24
//

#include "Shadows.h"
#include "GBuffer.h"
#include "Debug.h"

#include <imgui/imgui.h>
#include <DemoApp/Camera.h>

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

    // Create cascades and cascade buffer
    RHITextureDesc cascadeDesc;
    cascadeDesc.Width = SHADOW_CASCADE_QUALITY;
    cascadeDesc.Height = cascadeDesc.Width;
    cascadeDesc.Format = RHITextureFormat::kD32_FLOAT;
    cascadeDesc.Usage = RHITextureUsage::kDepthTarget | RHITextureUsage::kShaderResource;

    RendererResourceManager::CreateTexture(SHADOWS_CASCADE_0, cascadeDesc);
    RendererResourceManager::CreateTexture(SHADOWS_CASCADE_1, cascadeDesc);
    RendererResourceManager::CreateTexture(SHADOWS_CASCADE_2, cascadeDesc);
    RendererResourceManager::CreateTexture(SHADOWS_CASCADE_3, cascadeDesc);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCascadeBuffers[i] = mParentDevice->CreateBuffer(RHIBufferDesc(sizeof(ShadowCascade) * SHADOW_CASCADE_COUNT, sizeof(ShadowCascade), RHIBufferUsage::kStaging | RHIBufferUsage::kShaderRead));
        mCascadeBuffers[i]->SetName("Cascade Buffer");
    }

    // Create pipelines
    CODE_BLOCK("Create CSM resources") {
        CompiledShader shader = ShaderCompiler::Compile("Shadows/CSM.hlsl");
        CompiledShader shaderNoAlpha = ShaderCompiler::Compile("Shadows/CSMNoAlpha.hlsl");
        CompiledShader shaderPopulate = ShaderCompiler::Compile("Shadows/CSMPopulate.hlsl");

        RHIGraphicsPipelineDesc pipelineDesc = {};
        pipelineDesc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
        pipelineDesc.Bytecode[ShaderStage::kFragment] = shader.Entries["PSMain"];
        pipelineDesc.RenderTargetFormats = {};
        pipelineDesc.DepthEnabled = true;
        pipelineDesc.DepthWrite = true;
        pipelineDesc.DepthClampEnabled = true;
        pipelineDesc.DepthFormat = RHITextureFormat::kD32_FLOAT;
        pipelineDesc.DepthOperation = RHIDepthOperation::kLess;
        pipelineDesc.CullMode = RHICullMode::kBack;
        pipelineDesc.PushConstantSize = sizeof(uint) * 4 + sizeof(glm::mat4) * 2;
        mCSMPipeline = mParentDevice->CreateGraphicsPipeline(pipelineDesc);

        pipelineDesc.Bytecode[ShaderStage::kVertex] = shaderNoAlpha.Entries["VSMain"];
        pipelineDesc.Bytecode[ShaderStage::kFragment] = shaderNoAlpha.Entries["PSMain"];
        mCSMPipelineNoAlpha = mParentDevice->CreateGraphicsPipeline(pipelineDesc);

        mCSMToMaskPipeline = mParentDevice->CreateComputePipeline(RHIComputePipelineDesc(sizeof(uint) * 16, shaderPopulate.Entries["CSMain"]));
    }
    CODE_BLOCK("Create Hard RT resources") {
        CompiledShader shader = ShaderCompiler::Compile("Shadows/HardRT.hlsl");
        CompiledShader shaderNoAlpha = ShaderCompiler::Compile("Shadows/HardRTNoAlpha.hlsl");

        mHardRTShadows = mParentDevice->CreateComputePipeline(RHIComputePipelineDesc(sizeof(uint) * 12, shader.Entries["CSMain"]));
        mHardRTShadowsNoAlpha = mParentDevice->CreateComputePipeline(RHIComputePipelineDesc(sizeof(uint) * 12, shaderNoAlpha.Entries["CSMain"]));
    }
}

Shadows::~Shadows()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mCascadeBuffers[i];
    }
    delete mCSMToMaskPipeline;
    delete mCSMPipelineNoAlpha;
    delete mCSMPipeline;
    delete mHardRTShadowsNoAlpha;
    delete mHardRTShadows;
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

void Shadows::CSM(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("CSM");
    CODE_BLOCK("Update Cascade Matrices") {
        uint cascadeSize = SHADOW_CASCADE_QUALITY;
        Array<float> splits(SHADOW_CASCADE_COUNT + 1);

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
            Array<float4> corners = Math::GetFrustumCorners(begin.CamData.View, projection);

            // Calculate center
            float3 center(0.0f);
            for (const float4& corner : corners) {
                center += float3(corner);
            }
            center /= corners.size();

            // Adjust light's up vector
            float3 up(0.0f, 1.0f, 0.0f);
            if (glm::abs(glm::dot(begin.RenderScene->GetLights().Sun.Direction, up)) > 0.999f) {
                up = float3(1.0f, 0.0f, 0.0f);
            }

            // Calculate light-space bounding sphere
            float3 minBounds(FLT_MAX), maxBounds(-FLT_MAX);
            float sphereRadius = 0.0f;
            for (auto& corner : corners) {
                float dist = glm::length(float3(corner) - center);
                sphereRadius = std::max(sphereRadius, dist);
            }
            sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;
            maxBounds = float3(sphereRadius);
            minBounds = -maxBounds;

            // Get extents and create view matrix
            float3 cascadeExtents = maxBounds - minBounds;
            float3 shadowCameraPos = center - begin.RenderScene->GetLights().Sun.Direction;

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
            const char* indexToID[4] = { SHADOWS_CASCADE_0, SHADOWS_CASCADE_1,SHADOWS_CASCADE_2, SHADOWS_CASCADE_3 };
            RendererResource& depthTexture = RendererResourceManager::Get(indexToID[i]);

            if (!mUpdateCascades) {
                Debug::DrawFrustum(mCascades[i].Proj * mCascades[i].View);
            } else {
                mCascades[i].Split = splits[i + 1];
                mCascades[i].View = lightView;
                mCascades[i].Proj = lightProjection;
                mCascades[i].SRVIndex = RendererViewRecycler::GetTextureView(RHITextureViewDesc(depthTexture.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle();
            }
        }

        void* ptr = mCascadeBuffers[begin.FrameIndex]->Map();
        memcpy(ptr, mCascades.data(), mCascades.size() * sizeof(ShadowCascade));
        mCascadeBuffers[begin.FrameIndex]->Unmap();
    }
    CODE_BLOCK("Draw Cascades") {
        // Draw that shyte
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            begin.CommandList->PushMarker("Cascade " + std::to_string(i));

            IRHIGraphicsPipeline* pipeline = mAlphaTest ? mCSMPipeline : mCSMPipelineNoAlpha;
            const char* indexToID[4] = { SHADOWS_CASCADE_0, SHADOWS_CASCADE_1,SHADOWS_CASCADE_2, SHADOWS_CASCADE_3 };

            RendererResource& depthTexture = RendererResourceManager::Import(indexToID[i], begin.CommandList, RendererImportType::kDepthWrite);
            RendererResource& defaultWhite = RendererResourceManager::Get(DEFAULT_WHITE_TEXTURE);
            RendererResource& materialSampler = RendererResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
            RHIRenderBegin renderBegin(SHADOW_CASCADE_QUALITY, SHADOW_CASCADE_QUALITY, {}, RHIRenderAttachment(RendererViewRecycler::GetDSV(depthTexture.Texture)));

            begin.CommandList->BeginRendering(renderBegin);
            begin.CommandList->SetGraphicsPipeline(pipeline);
            begin.CommandList->SetViewport(SHADOW_CASCADE_QUALITY, SHADOW_CASCADE_QUALITY, 0, 0);
            for (auto& entity : begin.RenderScene->GetEntities()) {
                Model* model = entity.Model->Model;
                for (auto& node : model->GetNodes()) {
                    for (auto& primitive : node.Primitives) {
                        ModelMaterial material = model->GetMaterials()[primitive.MaterialIndex];
                        BindlessHandle albedoView = material.Albedo ? material.Albedo->TextureOrImage.View->GetBindlessHandle() : RendererViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle();

                        struct PushConstants {
                            BindlessHandle VertexBuffer;
                            BindlessHandle Albedo;
                            BindlessHandle Sampler;
                            uint Pad;

                            glm::mat4 View;
                            glm::mat4 Proj;
                        } constant = {
                            RendererViewRecycler::GetSRV(primitive.VertexBuffer)->GetBindlessHandle(),
                            albedoView,
                            materialSampler.Sampler->GetBindlessHandle(),
                            0,

                            mCascades[i].View,
                            mCascades[i].Proj
                        };

                        begin.CommandList->SetIndexBuffer(primitive.IndexBuffer);
                        begin.CommandList->SetGraphicsConstants(pipeline, &constant, sizeof(constant));
                        begin.CommandList->DrawIndexed(primitive.IndexCount, 1, 0, 0, 0);
                    }
                }
            }
            begin.CommandList->EndRendering();

            begin.CommandList->PopMarker();
        }
    }
    CODE_BLOCK("Populate visibility mask") {
        begin.CommandList->PushMarker("Populate visibility mask");

        // Transition to shader read
        const char* indexToID[4] = { SHADOWS_CASCADE_0, SHADOWS_CASCADE_1,SHADOWS_CASCADE_2, SHADOWS_CASCADE_3 };
        for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
            (void)RendererResourceManager::Import(indexToID[i], begin.CommandList, RendererImportType::kShaderRead);
        }

        RendererResource& output = RendererResourceManager::Import(SHADOWS_SUN_MASK_ID, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& gbufferDepth = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& gbufferNormal = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kShaderRead);
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        RendererResource& materialSampler = RendererResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);

        struct PushConstants {
            BindlessHandle CascadeIndex;
            BindlessHandle SamplerIndex;
            BindlessHandle CameraIndex;
            BindlessHandle DepthIndex;

            BindlessHandle OutputIndex;
            BindlessHandle SunIndex;
            BindlessHandle NormalIndex;
            uint Pad;

            uint Width;
            uint Height;
            uint2 Pad1;
        } constants = {
            RendererViewRecycler::GetSRV(mCascadeBuffers[begin.FrameIndex])->GetBindlessHandle(),
            materialSampler.Sampler->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            RendererViewRecycler::GetTextureView(RHITextureViewDesc(gbufferDepth.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle(),

            RendererViewRecycler::GetUAV(output.Texture)->GetBindlessHandle(),
            begin.RenderScene->GetLights().GetSunBufferView(begin.FrameIndex)->GetBindlessHandle(),
            RendererViewRecycler::GetSRV(gbufferNormal.Texture)->GetBindlessHandle(),
            0,

            mWidth,
            mHeight,
            uint2(0)
        };

        begin.CommandList->SetComputePipeline(mCSMToMaskPipeline);
        begin.CommandList->SetComputeConstants(mCSMToMaskPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch((mWidth + 7) / 8, (mHeight + 7) / 8, 1);

        begin.CommandList->PopMarker();
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
        const char* modes[] = { "None", "CSM (Experimental)", "Hard RT", "Soft RT" };
        ImGui::Combo("Shadow Technique", (int*)&mMode, modes, 4, 4);
        ImGui::Checkbox("Alpha Test", &mAlphaTest);

        switch (mMode) {
            case ShadowMode::kHardRT: {
                ImGui::SliderFloat("Normal Bias", &mNormalBias, 0.001f, 0.01f);
                break;
            }
            case ShadowMode::kCSM: {
                ImGui::SliderFloat("Shadow Split Lambda", &mSplitLambda, 0.01f, 0.99f);
                ImGui::Checkbox("Update Cascades", &mUpdateCascades);
                break;
            }
            default: {
                break;
            }
        }
        ImGui::TreePop();
    }
}
