//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 19:44:20
//

#include "Texture.h"

#include <Core/FileSystem.h>

void TextureAsset::Load(const std::string& path)
{
    if (!FileSystem::Exists(path)) {
        SERAPH_WARN("%s isn't compressed, skipping");
        return;
    }

    uint64 size = FileSystem::GetFileSize(path);
    Pixels.resize(size - sizeof(TextureHeader));

    FILE* f = fopen(path.c_str(), "rb+");
    fread(&Header, sizeof(TextureHeader), 1, f);
    fread(Pixels.data(), Pixels.size(), 1, f);
    fclose(f);

    SERAPH_WHATEVER("Read texture file %s", path.c_str());
}
