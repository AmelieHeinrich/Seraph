//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 14:25:34
//

#pragma once

#include <Core/Context.h>

constexpr uint64 TEXTURE_ROW_PITCH_ALIGNMENT = 256;

enum class RHITextureFormat
{
    kUndefined,
    kR8G8B8A8_UNORM,
    kR8G8B8A8_sRGB,
    kB8G8R8A8_UNORM,
    kR16G16B16A16_FLOAT,
    kR32_FLOAT,
    kD32_FLOAT,
    kBC7_UNORM,
    kR16G16_FLOAT
};

enum class RHITextureUsage : uint
{
    kRenderTarget = BIT(1),
    kDepthTarget = BIT(2),
    kStorage = BIT(3),
    kShaderResource = BIT(4)
};
ENUM_CLASS_FLAGS(RHITextureUsage)

enum class RHIResourceLayout
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

struct RHITextureDesc
{
    uint Width;
    uint Height;
    uint Depth = 1;
    uint MipLevels = 1;
    RHITextureFormat Format;
    RHITextureUsage Usage;
    bool Reserved; // Used by RHI for internal swapchain images
};

class IRHIDevice;

class IRHITexture
{
public:
    ~IRHITexture() = default;

    virtual void SetName(const String& name) = 0;

    RHITextureDesc GetDesc() const { return mDesc; }

    RHIResourceLayout GetLayout() { return mLayout; }
    void SetLayout(RHIResourceLayout layout) { mLayout = layout; }
public:
    static uint32 BytesPerPixel(RHITextureFormat format);
    static bool IsBlockFormat(RHITextureFormat format);

protected:
    RHITextureDesc mDesc;
    RHIResourceLayout mLayout = RHIResourceLayout::kUndefined;
};
