//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 15:25:48
//

#pragma once

#include <Core/Context.h>

#include "Bindless.h"
#include "Texture.h"

enum class RHITextureViewType
{
    kNone,
    kRenderTarget,
    kDepthTarget,
    kShaderRead,
    kShaderWrite
};

enum class RHITextureViewDimension
{
    kTexture2D,
    kTextureCube
};

constexpr uint64 VIEW_ALL_MIPS = 0xFFFFFFFF;

struct RHITextureViewDesc
{
    IRHITexture* Texture;
    RHITextureViewType Type;
    RHITextureViewDimension Dimension;
    RHITextureFormat ViewFormat;
    uint64 ViewMip = VIEW_ALL_MIPS;
    uint64 ArrayLayer = 0;

    RHITextureViewDesc() = default;

    RHITextureViewDesc(IRHITexture* texture, RHITextureViewType type)
        : Texture(texture), Type(type), Dimension(RHITextureViewDimension::kTexture2D)
    {
        RHITextureDesc desc = texture->GetDesc();
        ViewFormat = desc.Format;
    }

    RHITextureViewDesc(IRHITexture* texture, RHITextureViewType type, RHITextureFormat format)
        : Texture(texture), Type(type), Dimension(RHITextureViewDimension::kTexture2D), ViewFormat(format)
    {
    }
};

class IRHIDevice;

class IRHITextureView
{
public:
    ~IRHITextureView() = default;

    RHITextureViewDesc GetDesc() const { return mDesc; }
    BindlessHandle GetBindlessHandle() const { return mBindless; }

    virtual uint64 GetTextureID() = 0;
protected:
    RHITextureViewDesc mDesc;

    BindlessHandle mBindless;
};
