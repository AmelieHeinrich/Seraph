//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:05:54
//

#pragma once

#include "KGPU_Texture.h"
#include "KGPU_Bindless.h"

namespace KGPU
{
    enum class TextureViewType
    {
        kNone,
        kRenderTarget,
        kDepthTarget,
        kShaderRead,
        kShaderWrite
    };
    
    enum class TextureViewDimension
    {
        kTexture2D,
        kTextureCube
    };
    
    constexpr uint64 VIEW_ALL_MIPS = 0xFFFFFFFF;
    
    struct TextureViewDesc
    {
        ITexture* Texture = nullptr;
        TextureViewType Type = TextureViewType::kNone;
        TextureViewDimension Dimension = TextureViewDimension::kTexture2D;
        TextureFormat ViewFormat = TextureFormat::kUndefined;
        uint64 ViewMip = VIEW_ALL_MIPS;
        uint64 ArrayLayer = 0;
    
        TextureViewDesc() = default;
    
        TextureViewDesc(ITexture* texture, TextureViewType type)
            : Texture(texture), Type(type), Dimension(TextureViewDimension::kTexture2D)
        {
            TextureDesc desc = texture->GetDesc();
            ViewFormat = desc.Format;
        }
    
        TextureViewDesc(ITexture* texture, TextureViewType type, TextureFormat format)
            : Texture(texture), Type(type), Dimension(TextureViewDimension::kTexture2D), ViewFormat(format)
        {
        }
    };

    inline bool operator==(const TextureViewDesc& a, const TextureViewDesc& b) {
        return a.Texture == b.Texture &&
               a.Type == b.Type &&
               a.Dimension == b.Dimension &&
               a.ViewFormat == b.ViewFormat &&
               a.ViewMip == b.ViewMip &&
               a.ArrayLayer == b.ArrayLayer;
    }
    
    class IDevice;
    
    class ITextureView
    {
    public:
        virtual ~ITextureView() = default;
    
        TextureViewDesc GetDesc() const { return mDesc; }
        BindlessHandle GetBindlessHandle() const { return mBindless; }
    protected:
        TextureViewDesc mDesc;
    
        BindlessHandle mBindless;
    };
}
