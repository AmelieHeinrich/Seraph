//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 19:21:12
//

#include "SP_GBuffer.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_Uploader.h>
#include <Graphics/Gfx_Mipmapper.h>

#include <imgui.h>

namespace SP
{
    GBuffer::GBuffer()
    {
        // Textures
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        KGPU::TextureDesc depthDesc, normalDesc, albedoDesc, pbrDesc, motionDesc;
        depthDesc.Width = width;
        depthDesc.Height = height;
        depthDesc.Format = KGPU::TextureFormat::kD32_FLOAT;
        depthDesc.Usage = KGPU::TextureUsage::kDepthTarget | KGPU::TextureUsage::kShaderResource;

        normalDesc.Width = width;
        normalDesc.Height = height;
        normalDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        normalDesc.Usage = KGPU::TextureUsage::kRenderTarget | KGPU::TextureUsage::kShaderResource;

        albedoDesc.Width = width;
        albedoDesc.Height = height;
        albedoDesc.Format = KGPU::TextureFormat::kR8G8B8A8_UNORM;
        albedoDesc.Usage = KGPU::TextureUsage::kRenderTarget | KGPU::TextureUsage::kShaderResource;

        pbrDesc.Width = width;
        pbrDesc.Height = height;
        pbrDesc.Format = KGPU::TextureFormat::kR16G16_FLOAT;
        pbrDesc.Usage = KGPU::TextureUsage::kRenderTarget | KGPU::TextureUsage::kShaderResource;

        motionDesc.Width = width;
        motionDesc.Height = height;
        motionDesc.Format = KGPU::TextureFormat::kR16G16_FLOAT;
        motionDesc.Usage = KGPU::TextureUsage::kRenderTarget | KGPU::TextureUsage::kShaderResource;

        Gfx::ResourceManager::CreateTexture(GBUFFER_DEPTH_ID, depthDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_PREV_DEPTH_ID, depthDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_NORMAL_ID, normalDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_PREV_NORMAL_ID, normalDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_ALBEDO_ID, albedoDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_PBR_ID, pbrDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_MOTION_VECTOR_ID, motionDesc);
        Gfx::ResourceManager::CreateTexture(GBUFFER_EMISSIVE_ID, albedoDesc);
        Gfx::ResourceManager::CreateSampler(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID, KGPU::SamplerDesc(KGPU::SamplerAddress::kWrap, KGPU::SamplerFilter::kLinear, true));
        Gfx::ResourceManager::CreateSampler(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID, KGPU::SamplerDesc(KGPU::SamplerAddress::kWrap, KGPU::SamplerFilter::kNearest, true));

        // Create pipeline
        KGPU::GraphicsPipelineDesc desc;
        desc.DepthEnabled = true;
        desc.DepthWrite = true;
        desc.DepthOperation = KGPU::DepthOperation::kLess;
        desc.RenderTargetFormats = {
            normalDesc.Format,
            albedoDesc.Format,
            pbrDesc.Format,
            motionDesc.Format,
            albedoDesc.Format
        };
        Gfx::ShaderManager::SubscribeGraphics("data/sp/shaders/gbuffer.kds", desc);

        // Camera CBV
        Gfx::ResourceManager::CreateRingBuffer(GBUFFER_CAMERA_CBV_ID, KOS::Align<uint>(sizeof(CameraData), 256));
   
        // Grid
        KDA::Texture texture = KDA::TextureLoader::LoadFromFile("data/kd/textures/default.png", true);

        Gfx::ResourceManager::CreateTexture(DEFAULT_GRID_TEXTURE, texture.ToTextureDesc());
        Gfx::Uploader::EnqueueTextureUploadRaw(texture.Bytes.data(), texture.Bytes.size(), Gfx::ResourceManager::Get(DEFAULT_GRID_TEXTURE).Texture, false);
        Gfx::Mipmapper::ComputeMipmaps(Gfx::ResourceManager::Get(DEFAULT_GRID_TEXTURE).Texture);
    }

    void GBuffer::Render(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::GBuffer::Render");

        RenderScene(begin);
        CopyToHistory(begin);
    }

    void GBuffer::UI(RenderPassBegin& begin)
    {
        if (ImGui::TreeNodeEx("GBuffer", ImGuiTreeNodeFlags_Framed)) {
            static int currentChoice = 0; // 0 = White, 1 = Grid
            const char* items[] = { "White Texture", "Grid Texture" };

            if (ImGui::Combo("Default Texture", &currentChoice, items, IM_ARRAYSIZE(items))) {
                if (currentChoice == 0)
                    mDefaultTextureID = Gfx::DEFAULT_WHITE_TEXTURE;
                else
                    mDefaultTextureID = DEFAULT_GRID_TEXTURE;
            }

            ImGui::TreePop();
        }
    }

    void GBuffer::RenderScene(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::GBuffer::RenderScene");

        // Upload camera data
        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        void* ptr = cameraBuffer.RingBuffer[begin.FrameIndex]->Map();
        memcpy(ptr, &begin.CamData, sizeof(begin.CamData));
        cameraBuffer.RingBuffer[begin.FrameIndex]->Unmap();

        // Import everything
        Gfx::Resource& depthTexture = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kDepthWrite);
        Gfx::Resource& normalTexture = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        Gfx::Resource& albedoTexture = Gfx::ResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        Gfx::Resource& pbrTexture = Gfx::ResourceManager::Import(GBUFFER_PBR_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        Gfx::Resource& motionTexture = Gfx::ResourceManager::Import(GBUFFER_MOTION_VECTOR_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        Gfx::Resource& emissiveTexture = Gfx::ResourceManager::Import(GBUFFER_EMISSIVE_ID, begin.CmdList, Gfx::ImportType::kColorWrite);
        Gfx::Resource& materialSampler = Gfx::ResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
        Gfx::Resource& defaultWhite = Gfx::ResourceManager::Get(mDefaultTextureID);
        Gfx::Resource& defaultBlack = Gfx::ResourceManager::Get(Gfx::DEFAULT_BLACK_TEXTURE);

        // Begin render
        KC::Array<KGPU::RenderAttachment> attachments = {
            KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(normalTexture.Texture)),
            KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(albedoTexture.Texture)),
            KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(pbrTexture.Texture)),
            KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(motionTexture.Texture)),
            KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(emissiveTexture.Texture))
        };
        KGPU::RenderBegin renderBegin(begin.Width, begin.Height, attachments, KGPU::RenderAttachment(Gfx::ViewRecycler::GetDSV(depthTexture.Texture)));
        
        KGPU::IGraphicsPipeline* pipeline = Gfx::ShaderManager::GetGraphics("data/sp/shaders/gbuffer.kds");

        begin.CmdList->BeginRendering(renderBegin);
        begin.CmdList->SetGraphicsPipeline(pipeline);
        begin.CmdList->SetRenderSize(begin.Width, begin.Height);

        uint index = 0;
        begin.World->ForEach([&](RenderEntity entity, Gfx::Material* material){
            KGPU::BindlessHandle albedoView = material->GetAlbedoView() ? material->GetAlbedoView()->GetBindlessHandle() : Gfx::ViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle();
            KGPU::BindlessHandle normalView = material->GetNormalView() ? material->GetNormalView()->GetBindlessHandle() : KGPU::BINDLESS_INVALID_HANDLE;
            KGPU::BindlessHandle pbrView = material->GetMRView() ? material->GetMRView()->GetBindlessHandle() : KGPU::BINDLESS_INVALID_HANDLE;
            KGPU::BindlessHandle emissiveView = material->GetEmissiveView() ? material->GetEmissiveView()->GetBindlessHandle() : Gfx::ViewRecycler::GetSRV(defaultBlack.Texture)->GetBindlessHandle();

            struct PushConstant {
                KGPU::BindlessHandle VB;
                KGPU::BindlessHandle Albedo;
                KGPU::BindlessHandle Normal;
                KGPU::BindlessHandle PBR;

                KGPU::BindlessHandle Sampler;
                KGPU::BindlessHandle Camera;
                int Width;
                int Height;

                uint MeshIndex;
                KGPU::BindlessHandle Emissive;
                KGPU::float2 Pad;
            } constants = {
                entity.Primitive->GetVertexBufferView()->GetBindlessHandle(),
                albedoView,
                normalView,
                pbrView,

                materialSampler.Sampler->GetBindlessHandle(),
                cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
                begin.Width,
                begin.Height,

                index++,
                emissiveView,
                {}
            };

            begin.CmdList->SetIndexBuffer(entity.Primitive->GetIndexBuffer());
            begin.CmdList->SetGraphicsConstants(pipeline, &constants, sizeof(constants));
            begin.CmdList->DrawIndexed(entity.Primitive->GetIndexCount(), 1, 0, 0, 0);
        });
        begin.CmdList->EndRendering();
    
        if (Gfx::Manager::GetDevice()->SupportsRaytracing())
            begin.World->GetRTWorld()->Build(begin.CmdList);
    }

    void GBuffer::CopyToHistory(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::GBuffer::CopyToHistory");
    
        Gfx::Resource& depthTexture = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kTransferSource);
        Gfx::Resource& normalTexture = Gfx::ResourceManager::Import(GBUFFER_NORMAL_ID, begin.CmdList, Gfx::ImportType::kTransferSource);
        Gfx::Resource& prevDepthTexture = Gfx::ResourceManager::Import(GBUFFER_PREV_DEPTH_ID, begin.CmdList, Gfx::ImportType::kTransferDest);
        Gfx::Resource& prevNormalTexture = Gfx::ResourceManager::Import(GBUFFER_PREV_NORMAL_ID, begin.CmdList, Gfx::ImportType::kTransferDest);
    
        begin.CmdList->CopyTextureToTexture(prevDepthTexture.Texture, depthTexture.Texture);
        begin.CmdList->CopyTextureToTexture(prevNormalTexture.Texture, normalTexture.Texture);
    }
}
