//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 13:22:51
//

#include "RendererResourceManager.h"

RendererResourceManager::Data RendererResourceManager::sData;

void RendererResourceManager::Initialize(IRHIDevice* device)
{
    sData.Device = device;
}

void RendererResourceManager::Shutdown()
{
    sData.Resources.clear();
}

void RendererResourceManager::CreateTexture(const String& name, RHITextureDesc desc)
{
    RendererResource resource;
    resource.Type = RendererResourceType::kTexture;
    resource.Texture = sData.Device->CreateTexture(desc);
    resource.Texture->SetName(name);
    sData.Resources[name] = std::move(resource);
}

void RendererResourceManager::CreateBuffer(const String& name, RHIBufferDesc desc)
{
    RendererResource resource;
    resource.Type = RendererResourceType::kBuffer;
    resource.Buffer = sData.Device->CreateBuffer(desc);
    resource.Buffer->SetName(name);
    sData.Resources[name] = std::move(resource);
}

void RendererResourceManager::CreateRingBuffer(const String& name, uint size)
{
    RendererResource resource;
    resource.Type= RendererResourceType::kRingBuffer;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        resource.RingBuffer[i] = sData.Device->CreateBuffer(RHIBufferDesc(size, 0, RHIBufferUsage::kConstant));
        resource.RingBufferViews[i] = sData.Device->CreateBufferView(RHIBufferViewDesc(resource.RingBuffer[i], RHIBufferViewType::kConstant));
        resource.RingBuffer[i]->SetName(name);
    }
    sData.Resources[name] = std::move(resource);
}

void RendererResourceManager::CreateSampler(const String& name, RHISamplerDesc desc)
{
    RendererResource resource;
    resource.Type = RendererResourceType::kSampler;
    resource.Sampler = sData.Device->CreateSampler(desc);
    sData.Resources[name] = std::move(resource);
}

RendererResource& RendererResourceManager::Get(const String& name)
{
    return sData.Resources[name];
}

RendererResource& RendererResourceManager::Import(const String& name, IRHICommandList* list, RendererImportType type)
{
    RendererResource& resource = sData.Resources[name];
    switch (resource.Type)
    {
    case RendererResourceType::kBuffer: {
        RHIBufferBarrier barrier(resource.Buffer);
        barrier.SourceAccess = resource.LastAccess;
        barrier.SourceStage = resource.LastStage;

        switch (type) {
            case RendererImportType::kColorWrite: {
                SERAPH_WARN("Can't use import type color write on buffer!");
                return resource;
            }
            case RendererImportType::kDepthWrite: {
                SERAPH_WARN("Can't use import type depth write on buffer!");
                return resource;
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

        list->Barrier(barrier);
        break;
    }
    case RendererResourceType::kTexture: {
        RHITextureBarrier barrier(resource.Texture);
        barrier.SourceAccess = resource.LastAccess;
        barrier.SourceStage = resource.LastStage;
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

        list->Barrier(barrier);
        break;
    }
    }
    return resource;
}
