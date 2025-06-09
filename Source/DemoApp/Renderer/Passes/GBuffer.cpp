//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:55:31
//

#include "GBuffer.h"

GBuffer::GBuffer(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Textures
    RHITextureDesc depthDesc, normalDesc, albedoDesc;
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

    RendererResourceManager::CreateTexture(GBUFFER_DEPTH_ID, depthDesc);
    RendererResourceManager::CreateTexture(GBUFFER_NORMAL_ID, normalDesc);
    RendererResourceManager::CreateTexture(GBUFFER_ALBEDO_ID, albedoDesc);
    RendererResourceManager::CreateSampler(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID, RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kLinear, true));
    RendererResourceManager::CreateSampler(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID, RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kNearest, true));

    // Model
    mSponza = AssetManager::Get("Data/Models/Sponza/Sponza.gltf", AssetType::kModel);

    // Shader
    CompiledShader shader = ShaderCompiler::Compile("GBuffer", { "VSMain", "FSMain" });

    RHIGraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    pipelineDesc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    pipelineDesc.PushConstantSize = sizeof(BindlessHandle) * 4 + sizeof(glm::mat4) * 2;
    pipelineDesc.RenderTargetFormats = {
        RHITextureFormat::kR16G16B16A16_FLOAT,
        RHITextureFormat::kR8G8B8A8_UNORM
    };
    pipelineDesc.DepthEnabled = true;
    pipelineDesc.DepthWrite = true;
    pipelineDesc.DepthFormat = RHITextureFormat::kD32_FLOAT;
    pipelineDesc.DepthOperation = RHIDepthOperation::kLess;

    mPipeline = mParentDevice->CreateGraphicsPipeline(pipelineDesc);
}

GBuffer::~GBuffer()
{
    AssetManager::Release(mSponza);
    delete mPipeline;
}

void GBuffer::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("GBuffer");
    {
        RendererResource& depthTexture = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kDepthWrite);
        RendererResource& normalTexture = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kColorWrite);
        RendererResource& albedoTexture = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kColorWrite);
        RendererResource& materialSampler = RendererResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
        
        Array<RHIRenderAttachment> attachments = {
            RHIRenderAttachment(RendererViewRecycler::GetRTV(normalTexture.Texture)),
            RHIRenderAttachment(RendererViewRecycler::GetRTV(albedoTexture.Texture))
        };
        RHIRenderBegin renderBegin(mWidth, mHeight, attachments, RHIRenderAttachment(RendererViewRecycler::GetDSV(depthTexture.Texture)));
    
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->SetGraphicsPipeline(mPipeline);
        begin.CommandList->SetViewport(mWidth, mHeight, 0, 0);
        for (auto& node : mSponza->Model->GetNodes()) {
            for (auto& primitive : node.Primitives) {
                ModelMaterial material = mSponza->Model->GetMaterials()[primitive.MaterialIndex];
            
                struct PushConstant {
                    BindlessHandle Texture;
                    BindlessHandle Sampler;
                    BindlessHandle VertexBuffer;
                    uint Pad;
                
                    glm::mat4 View;
                    glm::mat4 Projection;
                } constant = {
                    material.Albedo->TextureOrImage.View->GetBindlessHandle(),
                    materialSampler.Sampler->GetBindlessHandle(),
                    RendererViewRecycler::GetSRV(primitive.VertexBuffer)->GetBindlessHandle(),
                    0,
                
                    begin.View,
                    begin.Projection
                };
            
                begin.CommandList->SetIndexBuffer(primitive.IndexBuffer);
                begin.CommandList->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
                begin.CommandList->DrawIndexed(primitive.IndexCount, 1, 0, 0, 0);
            }
        }
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}
