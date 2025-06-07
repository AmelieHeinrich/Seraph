//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 13:07:23
//

#pragma once

#include "RendererResource.h"

enum class RendererImportType
{
    kShaderRead,
    kShaderWrite,
    kDepthWrite,
    kColorWrite,
    kTransferSource,
    kTransferDest
};

class RendererResourceManager
{
public:
    static void Initialize(IRHIDevice* device);
    static void Shutdown();

    static void CreateTexture(const String& name, RHITextureDesc desc);
    static void CreateBuffer(const String& name, RHIBufferDesc desc);
    static void CreateRingBuffer(const String& name, uint size);
    static void CreateSampler(const String& name, RHISamplerDesc desc);

    static RendererResource& Get(const String& name);
    static RendererResource& Import(const String& name, IRHICommandList* list, RendererImportType type);
private:
    static struct Data {
        IRHIDevice* Device;
        UnorderedMap<String, RendererResource> Resources;
    } sData;
};
