//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 14:25:34
//

#pragma once

#include <Core/Context.h>

enum class RHITextureFormat
{
    kR8G8B8A8_UNORM,
    kR8G8B8A8_sRGB
};

enum class RHITextureUsage
{
    kRenderTarget = BIT(1),
    kDepthTarget = BIT(2),
    kStorage = BIT(3),
    kShaderResource = BIT(4)
};

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

// Operator shit

inline constexpr bool operator&(RHITextureUsage x, RHITextureUsage y)
{
    return static_cast<uint>(x) & static_cast<uint>(y);
}

inline constexpr RHITextureUsage operator|(RHITextureUsage x, RHITextureUsage y)
{
    return static_cast<RHITextureUsage>(static_cast<uint>(x) | static_cast<uint>(y));
}

inline constexpr RHITextureUsage operator^(RHITextureUsage x, RHITextureUsage y)
{
    return static_cast<RHITextureUsage>(static_cast<uint>(x) ^ static_cast<uint>(y));
}

inline constexpr RHITextureUsage operator~(RHITextureUsage x)
{
    return static_cast<RHITextureUsage>(~static_cast<uint>(x));
}

inline RHITextureUsage& operator|=(RHITextureUsage & x, RHITextureUsage y)
{
    x = x | y;
    return x;
}

inline RHITextureUsage& operator^=(RHITextureUsage & x, RHITextureUsage y)
{
    x = x ^ y;
    return x;
}
