//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 13:07:23
//

#pragma once

#include "RendererResource.h"

constexpr const char* DEFAULT_WHITE_TEXTURE = "Default/White";
constexpr const char* DEFAULT_BLACK_TEXTURE = "Default/Black";

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

    static void CreateTexture(const std::string& name, RHITextureDesc desc);
    static void CreateBuffer(const std::string& name, RHIBufferDesc desc);
    static void CreateRingBuffer(const std::string& name, uint size);
    static void CreateSampler(const std::string& name, RHISamplerDesc desc);

    static RendererResource& Get(const std::string& name);
    static RendererResource& Import(const std::string& name, IRHICommandList* list, RendererImportType type);
private:
    static struct Data {
        IRHIDevice* Device;
        UnorderedMap<std::string, RendererResource*> Resources;
    } sData;
};
