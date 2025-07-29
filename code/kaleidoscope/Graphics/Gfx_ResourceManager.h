//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 14:37:53
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

namespace Gfx
{
    constexpr const char* DEFAULT_WHITE_TEXTURE = "Default/White";
    constexpr const char* DEFAULT_BLACK_TEXTURE = "Default/Black";

    enum class ResourceType
    {
        kBuffer,
        kRingBuffer,
        kTexture,
        kSampler
    };

    struct Resource
    {
        ResourceType Type;
        KC::String Name;

        KGPU::IBuffer* Buffer = nullptr;
        KGPU::IBuffer* RingBuffer[KGPU::FRAMES_IN_FLIGHT];
        KGPU::IBufferView* RingBufferViews[KGPU::FRAMES_IN_FLIGHT];
        KGPU::ITexture* Texture = nullptr;
        KGPU::ISampler* Sampler = nullptr;

        KGPU::ResourceAccess LastAccess = KGPU::ResourceAccess::kNone;
        KGPU::PipelineStage LastStage = KGPU::PipelineStage::kNone;

        ~Resource();
    };

    enum class ImportType
    {
        kShaderRead,
        kShaderWrite,
        kDepthWrite,
        kColorWrite,
        kTransferSource,
        kTransferDest
    };

    class ResourceManager
    {
    public:
        static void Initialize();
        static void Shutdown();

        static void CreateTexture(const KC::String& name, KGPU::TextureDesc desc);
        static void CreateBuffer(const KC::String& name, KGPU::BufferDesc desc);
        static void CreateRingBuffer(const KC::String& name, uint size);
        static void CreateSampler(const KC::String& name, KGPU::SamplerDesc desc);

        static Resource& Get(const KC::String& name);
        static Resource& Import(const KC::String& name, KGPU::ICommandList* list, ImportType type);
    private:
        static struct Data {
            KC::HashMap<KC::String, Resource*> Resources;
        } sData;
    };
}
