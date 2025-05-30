//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 14:25:34
//

#pragma once

#include <Core/Context.h>

enum class RHITextureFormat
{
    kR8G8B8A8_UNORM,
    kR8G8B8A8_sRGB,
    kB8G8R8A8_UNORM,
    kD32_FLOAT
};

enum class RHITextureUsage : uint
{
    kRenderTarget = BIT(1),
    kDepthTarget = BIT(2),
    kStorage = BIT(3),
    kShaderResource = BIT(4)
};
ENUM_CLASS_FLAGS(RHITextureUsage)

struct RHITextureDesc
{
    uint Width;
    uint Height;
    uint Depth;
    uint MipLevels;
    RHITextureFormat Format;
    RHITextureUsage Usage;
    bool Reserved; // Used by RHI for internal swapchain images
};

class IRHIDevice;

class IRHITexture
{
public:
    ~IRHITexture() = default;

    virtual void SetName(const StringView& name) = 0;

    RHITextureDesc GetDesc() const { return mDesc; }

protected:
    RHITextureDesc mDesc;
};
