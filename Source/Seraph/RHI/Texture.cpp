//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 17:44:17
//

#include "Texture.h"

uint32 IRHITexture::BytesPerPixel(RHITextureFormat format)
{
    switch (format)
    {
    case RHITextureFormat::kR8G8B8A8_UNORM:   return 4; // 4 bytes per pixel (RGBA8)
    case RHITextureFormat::kR8G8B8A8_sRGB:    return 4;
    case RHITextureFormat::kB8G8R8A8_UNORM:   return 4;
    case RHITextureFormat::kR16G16B16A16_FLOAT: return 8; // 2 bytes * 4 channels
    case RHITextureFormat::kR32_FLOAT:         return 4; // single float
    case RHITextureFormat::kD32_FLOAT:         return 4; // depth 32-bit float
    case RHITextureFormat::kBC7_UNORM:         return 16; // block compressed: 16 bytes per 4x4 block
    }
    return 4;
}


bool IRHITexture::IsBlockFormat(RHITextureFormat format)
{
    if (format == RHITextureFormat::kBC7_UNORM)
        return true;
    return false;
}
