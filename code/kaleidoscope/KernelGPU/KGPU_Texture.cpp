//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:38:31
//

#include "KGPU_Texture.h"

namespace KGPU
{
    uint32 ITexture::BytesPerPixel(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::kR8G8B8A8_UNORM:     return 4; // 4 bytes per pixel (RGBA8)
        case TextureFormat::kR8G8B8A8_sRGB:      return 4;
        case TextureFormat::kB8G8R8A8_UNORM:     return 4;
        case TextureFormat::kR16G16B16A16_FLOAT: return 8; // 2 bytes * 4 channels
        case TextureFormat::kR16G16B16A16_UNORM: return 8;
        case TextureFormat::kR32_FLOAT:          return 4; // single float
        case TextureFormat::kD32_FLOAT:          return 4; // depth 32-bit float
        case TextureFormat::kBC7_UNORM:          return 16; // block compressed: 16 bytes per 4x4 block
        case TextureFormat::kBC7_sRGB:           return 16;
        case TextureFormat::kR16G16_FLOAT:       return 8;
        default: return 0;
        }
        return 4;
    }


    bool ITexture::IsBlockFormat(TextureFormat format)
    {
        if (format == TextureFormat::kBC7_UNORM)
            return true;
        return false;
    }

    TextureFormat ITexture::ToSRGB(TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::kR8G8B8A8_UNORM:     return TextureFormat::kR8G8B8A8_sRGB;
        case TextureFormat::kBC7_UNORM:          return TextureFormat::kBC7_sRGB;
        default: return format;
        }
        return format;
    }
}
