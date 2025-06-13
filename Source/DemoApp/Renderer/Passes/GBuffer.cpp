//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:55:31
//

#include "GBuffer.h"

GBuffer::GBuffer(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Textures
    RHITextureDesc depthDesc, normalDesc, albedoDesc, pbrDesc;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.Format = RHITextureFormat::kD32_FLOAT;
    depthDesc.Usage = RHITextureUsage::kDepthTarget | RHITextureUsage::kShaderResource;

    normalDesc.Width = width;
    normalDesc.Height = height;
    normalDesc.Format = RHITextureFormat::kR16G16B16A16_FLOAT;
    normalDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource;

    albedoDesc.Width = width;
    albedoDesc.Height = height;
    albedoDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
    albedoDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource;

    pbrDesc.Width = width;
    pbrDesc.Height = height;
    pbrDesc.Format = RHITextureFormat::kR16G16_FLOAT;
    pbrDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource;

    RendererResourceManager::CreateTexture(GBUFFER_DEPTH_ID, depthDesc);
    RendererResourceManager::CreateTexture(GBUFFER_NORMAL_ID, normalDesc);
    RendererResourceManager::CreateTexture(GBUFFER_ALBEDO_ID, albedoDesc);
    RendererResourceManager::CreateTexture(GBUFFER_PBR_ID, pbrDesc);
    RendererResourceManager::CreateSampler(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID, RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kLinear, true));
    RendererResourceManager::CreateSampler(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID, RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kNearest, true));

    // Shader
    CompiledShader shader = ShaderCompiler::Compile("GBuffer", { "VSMain", "FSMain" });

    RHIGraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    pipelineDesc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    pipelineDesc.PushConstantSize = sizeof(BindlessHandle) * 8 + sizeof(glm::mat4) * 2;
    pipelineDesc.RenderTargetFormats = {
        normalDesc.Format,
        albedoDesc.Format,
        pbrDesc.Format
    };
    pipelineDesc.DepthEnabled = true;
    pipelineDesc.DepthWrite = true;
    pipelineDesc.DepthFormat = RHITextureFormat::kD32_FLOAT;
    pipelineDesc.DepthOperation = RHIDepthOperation::kLess;

    mPipeline = mParentDevice->CreateGraphicsPipeline(pipelineDesc);

    // Camera CBV
    RendererResourceManager::CreateRingBuffer(GBUFFER_CAMERA_CBV_ID, Align<uint>(sizeof(CameraData), 256));
}

GBuffer::~GBuffer()
{
    delete mPipeline;
}

void GBuffer::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("GBuffer");
    {
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        void* ptr = cameraBuffer.RingBuffer[begin.FrameIndex]->Map();
        memcpy(ptr, &begin.CamData, sizeof(begin.CamData));
        cameraBuffer.RingBuffer[begin.FrameIndex]->Unmap();

        RendererResource& depthTexture = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kDepthWrite);
        RendererResource& normalTexture = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kColorWrite);
        RendererResource& albedoTexture = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kColorWrite);
        RendererResource& pbrTexture = RendererResourceManager::Import(GBUFFER_PBR_ID, begin.CommandList, RendererImportType::kColorWrite);

        RendererResource& materialSampler = RendererResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
        RendererResource& defaultWhite = RendererResourceManager::Get(DEFAULT_WHITE_TEXTURE);

        Array<RHIRenderAttachment> attachments = {
            RHIRenderAttachment(RendererViewRecycler::GetRTV(normalTexture.Texture)),
            RHIRenderAttachment(RendererViewRecycler::GetRTV(albedoTexture.Texture)),
            RHIRenderAttachment(RendererViewRecycler::GetRTV(pbrTexture.Texture))
        };
        RHIRenderBegin renderBegin(mWidth, mHeight, attachments, RHIRenderAttachment(RendererViewRecycler::GetDSV(depthTexture.Texture)));
    
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->SetGraphicsPipeline(mPipeline);
        begin.CommandList->SetViewport(mWidth, mHeight, 0, 0);
        for (auto& entity : begin.RenderScene->GetEntities()) {
            Model* model = entity.Model->Model;
            for (auto& node : model->GetNodes()) {
                for (auto& primitive : node.Primitives) {
                    ModelMaterial material = model->GetMaterials()[primitive.MaterialIndex];
                    BindlessHandle albedoView = material.Albedo ? material.Albedo->TextureOrImage.View->GetBindlessHandle() : RendererViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle();
                    BindlessHandle normalView = material.Normal ? material.Normal->TextureOrImage.View->GetBindlessHandle() : INVALID_HANDLE;
                    BindlessHandle pbrView = material.PBR ? material.PBR->TextureOrImage.View->GetBindlessHandle() : INVALID_HANDLE;

                    struct PushConstant {
                        BindlessHandle Albedo;
                        BindlessHandle Normal;
                        BindlessHandle PBR;
                        BindlessHandle Sampler;
                    
                        BindlessHandle VertexBuffer;
                        uint pad[3];

                        glm::mat4 View;
                        glm::mat4 Projection;
                    } constant = {
                        albedoView,
                        normalView,
                        pbrView,
                        materialSampler.Sampler->GetBindlessHandle(),

                        RendererViewRecycler::GetSRV(primitive.VertexBuffer)->GetBindlessHandle(),
                        {0,0,0},

                        begin.CamData.View,
                        begin.CamData.Proj
                    };
                
                    begin.CommandList->SetIndexBuffer(primitive.IndexBuffer);
                    begin.CommandList->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
                    begin.CommandList->DrawIndexed(primitive.IndexCount, 1, 0, 0, 0);
                }
            }
        }
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}
