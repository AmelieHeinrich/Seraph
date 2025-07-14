//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 13:22:51
//

#include "RendererResourceManager.h"

#include <RHI/Uploader.h>

RendererResourceManager::Data RendererResourceManager::sData;

void RendererResourceManager::Initialize(IRHIDevice* device)
{
    sData.Device = device;
    sData.Resources.clear();

    RHITextureDesc defaultDesc = {};
    defaultDesc.Width = 1;
    defaultDesc.Height = 1;
    defaultDesc.MipLevels = 1;
    defaultDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
    defaultDesc.Usage = RHITextureUsage::kShaderResource;

    CreateTexture(DEFAULT_WHITE_TEXTURE, defaultDesc);
    CreateTexture(DEFAULT_BLACK_TEXTURE, defaultDesc);

    auto& black = Get(DEFAULT_WHITE_TEXTURE);
    auto& white = Get(DEFAULT_BLACK_TEXTURE);

    uint32 blackColor = 0x000000FF;
    uint32 whiteColor = 0xFFFFFFFF;

    Uploader::EnqueueTextureUploadRaw(&blackColor, sizeof(uint), black.Texture);
    Uploader::EnqueueTextureUploadRaw(&whiteColor, sizeof(uint), white.Texture);
}

void RendererResourceManager::Shutdown()
{
    sData.Resources.clear();
}

void RendererResourceManager::CreateTexture(const std::string& name, RHITextureDesc desc)
{
    auto resource = std::make_shared<RendererResource>();
    resource->Name = name;
    resource->Type = RendererResourceType::kTexture;
    resource->Texture = sData.Device->CreateTexture(std::move(desc));
    resource->Texture->SetName(name);
    sData.Resources[name] = resource;
}

void RendererResourceManager::CreateBuffer(const std::string& name, RHIBufferDesc desc)
{
    auto resource = std::make_shared<RendererResource>();
    resource->Name = name;
    resource->Type = RendererResourceType::kBuffer;
    resource->Buffer = sData.Device->CreateBuffer(std::move(desc));
    resource->Buffer->SetName(name);
    sData.Resources[name] = resource;
}

void RendererResourceManager::CreateRingBuffer(const std::string& name, uint size)
{
    auto resource = std::make_shared<RendererResource>();
    resource->Name = name;
    resource->Type = RendererResourceType::kRingBuffer;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        resource->RingBuffer[i] = sData.Device->CreateBuffer(RHIBufferDesc(size, 0, RHIBufferUsage::kConstant));
        resource->RingBufferViews[i] = sData.Device->CreateBufferView(RHIBufferViewDesc(resource->RingBuffer[i], RHIBufferViewType::kConstant));
        resource->RingBuffer[i]->SetName(name + "_frame_" + std::to_string(i));
    }
    sData.Resources[name] = resource;
}

void RendererResourceManager::CreateSampler(const std::string& name, RHISamplerDesc desc)
{
    auto resource = std::make_shared<RendererResource>();
    resource->Name = name;
    resource->Type = RendererResourceType::kSampler;
    resource->Sampler = sData.Device->CreateSampler(std::move(desc));
    sData.Resources[name] = resource;
}

RendererResource& RendererResourceManager::Get(const std::string& name)
{
    return *sData.Resources[name];
}

RendererResource& RendererResourceManager::Import(const std::string& name, IRHICommandList* list, RendererImportType type)
{
    SharedPtr<RendererResource> resource = sData.Resources[name];
    switch (resource->Type)
    {
    case RendererResourceType::kBuffer: {
        RHIBufferBarrier barrier(resource->Buffer);
        barrier.SourceAccess = resource->LastAccess;
        barrier.SourceStage = resource->LastStage;

        switch (type) {
            case RendererImportType::kColorWrite: {
                SERAPH_WARN("Can't use import type color write on buffer!");
                return *resource;
            }
            case RendererImportType::kDepthWrite: {
                SERAPH_WARN("Can't use import type depth write on buffer!");
                return *resource;
            }
            case RendererImportType::kShaderRead: {
                barrier.DestAccess = RHIResourceAccess::kShaderRead;
                barrier.DestStage = RHIPipelineStage::kAllGraphics;
                break;
            }
            case RendererImportType::kShaderWrite: {
                barrier.DestAccess = RHIResourceAccess::kShaderWrite;
                barrier.DestStage = RHIPipelineStage::kAllGraphics;
                break;
            }
            case RendererImportType::kTransferSource: {
                barrier.DestAccess = RHIResourceAccess::kTransferRead;
                barrier.DestStage = RHIPipelineStage::kCopy;
                break;
            }
            case RendererImportType::kTransferDest: {
                barrier.DestAccess = RHIResourceAccess::kTransferWrite;
                barrier.DestStage = RHIPipelineStage::kCopy;
                break;
            }
        }

        resource->LastAccess = barrier.DestAccess,
        resource->LastStage = barrier.DestStage;

        list->Barrier(barrier);
        break;
    }
    case RendererResourceType::kTexture: {
        RHITextureBarrier barrier(resource->Texture);
        barrier.SourceAccess = resource->LastAccess;
        barrier.SourceStage = resource->LastStage;
        switch (type) {
            case RendererImportType::kColorWrite: {
                barrier.DestAccess = RHIResourceAccess::kColorAttachmentWrite;
                barrier.DestStage = RHIPipelineStage::kColorAttachmentOutput;
                barrier.NewLayout = RHIResourceLayout::kColorAttachment;
                break;
            }
            case RendererImportType::kDepthWrite: {
                barrier.DestAccess = RHIResourceAccess::kDepthStencilWrite;
                barrier.DestStage = RHIPipelineStage::kEarlyFragmentTests;
                barrier.NewLayout = RHIResourceLayout::kDepthStencilWrite;
                break;
            }
            case RendererImportType::kShaderRead: {
                barrier.DestAccess = RHIResourceAccess::kShaderRead;
                barrier.DestStage = RHIPipelineStage::kAllGraphics;
                barrier.NewLayout = RHIResourceLayout::kReadOnly;
                break;
            }
            case RendererImportType::kShaderWrite: {
                barrier.DestAccess = RHIResourceAccess::kShaderWrite;
                barrier.DestStage = RHIPipelineStage::kAllGraphics;
                barrier.NewLayout = RHIResourceLayout::kGeneral;
                break;
            }
            case RendererImportType::kTransferSource: {
                barrier.DestAccess = RHIResourceAccess::kTransferRead;
                barrier.DestStage = RHIPipelineStage::kCopy;
                barrier.NewLayout = RHIResourceLayout::kTransferSrc;
                break;
            }
            case RendererImportType::kTransferDest: {
                barrier.DestAccess = RHIResourceAccess::kTransferWrite;
                barrier.DestStage = RHIPipelineStage::kCopy;
                barrier.NewLayout = RHIResourceLayout::kTransferDst;
                break;
            }
        }

        resource->LastAccess = barrier.DestAccess,
        resource->LastStage = barrier.DestStage;

        list->Barrier(barrier);
        break;
    }
    default: break;
    }
    return *resource;
}
