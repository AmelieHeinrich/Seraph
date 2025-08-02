//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:04:59
//

#pragma once

#include <KernelCore/KC_Context.h>

namespace KGPU
{
    enum class TextureFormat
    {
        kUndefined,
        kR8G8B8A8_UNORM,
        kR8G8B8A8_sRGB,
        kB8G8R8A8_UNORM,
        kR16G16B16A16_FLOAT,
        kR16G16B16A16_UNORM,
        kR32_FLOAT,
        kR16_FLOAT,
        kR16_UINT,
        kR8_UINT,
        kD32_FLOAT,
        kBC7_UNORM,
        kBC7_sRGB,
        kR16G16_FLOAT
    };
    
    enum class TextureUsage : uint
    {
        kRenderTarget = BIT(1),
        kDepthTarget = BIT(2),
        kStorage = BIT(3),
        kShaderResource = BIT(4)
    };
    ENUM_CLASS_FLAGS(TextureUsage)
    
    enum class ResourceLayout
    {
        kUndefined,
        kGeneral,                 // UAV or equivalent
        kReadOnly,                // SRV/Texture in fragment or compute
        kColorAttachment,         // RenderTarget
        kDepthStencilReadOnly,
        kDepthStencilWrite,
        kTransferSrc,
        kTransferDst,
        kPresent,
    };
    
    struct TextureDesc
    {
        uint Width;
        uint Height;
        uint Depth = 1;
        uint MipLevels = 1;
        TextureFormat Format;
        TextureUsage Usage;
        bool Reserved = false; // Used by  for internal swapchain images
    };
    
    class ITexture
    {
    public:
        virtual ~ITexture() = default;
    
        virtual void SetName(const KC::String& name) = 0;
    
        TextureDesc GetDesc() const { return mDesc; }
    
        ResourceLayout GetLayout(uint mip = 0) { return mLayouts[mip]; }
        void SetLayout(ResourceLayout layout, uint mip = 0) { mLayouts[mip] = layout; }
    public:
        static uint32 BytesPerPixel(TextureFormat format);
        static bool IsBlockFormat(TextureFormat format);
        static TextureFormat ToSRGB(TextureFormat format);
    
    protected:
        TextureDesc mDesc;
        KC::Array<ResourceLayout> mLayouts;
    };
}
