//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 19:42:13
//

#pragma once

#include <RHI/Device.h>

struct TextureHeader
{
    uint Width;
    uint Height;
    uint Mips;
    RHITextureFormat Format;
};

struct TextureAsset
{
    TextureHeader Header;
    std::vector<uint8> Pixels;

    void Load(const std::string& path);
};
