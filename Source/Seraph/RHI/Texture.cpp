//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 17:44:17
//

#include "Texture.h"

uint32 IRHITexture::BytesPerPixel(RHITextureFormat format)
{
    switch (format)
    {
        case RHITextureFormat::kB8G8R8A8_UNORM: return 4;
        case RHITextureFormat::kR8G8B8A8_sRGB: return 4;
        case RHITextureFormat::kR8G8B8A8_UNORM: return 4;
    }
    return 4;
}

bool IRHITexture::IsBlockFormat(RHITextureFormat format)
{
    return false;
}
