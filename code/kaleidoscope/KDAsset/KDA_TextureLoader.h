//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 13:58:25
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

namespace KDA
{
    enum class TextureType
    {
        UncompressedImage,
        UncompressedFloatImage,
        Compressed
    };

    struct Texture
    {
        int Width;
        int Height;
        int MipCount;
        KGPU::TextureFormat Format;
        TextureType Type;
        KC::Array<uint8> Bytes;

        KGPU::TextureDesc ToTextureDesc();
    };

    class TextureLoader
    {
    public:
        static Texture LoadFromFile(const KC::String& path, bool srgb = false);
        // TODO: Load from memory stream

    };
}
