//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-11 16:11:34
//

// Use compressonator from AMD or something :)

#include "Asset/Compressor.h"
#include "Asset/Texture.h"

#include <filesystem>
#include <Core/FileSystem.h>

void Compressor::RecurseFolder(const std::string& path)
{
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path)) {
        std::string entryPath = dirEntry.path().string();
        std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
    
        if (dirEntry.path().extension() == ".png" || dirEntry.path().extension() == ".jpg") {
            CompressTexture(entryPath);
        }
    }
}

void Compressor::CompressTexture(const std::string& path)
{
    if (FileSystem::Exists(ToCachedPath(path))) {
        SERAPH_WHATEVER("Skipping %s", path.c_str());
        return;
    }
}

std::string Compressor::ToCachedPath(const std::string& path)
{
    const uint64 m = 0xc6a4a7935bd1e995ULL;
    const uint32 r = 47;

    uint64 h = 1000 ^ (path.size() * m);
    const uint64 * data = (const uint64*)path.data();
    const uint64 * end = data + (path.size() / 8);
    while (data != end) {
        uint64 k = *data++;
        k *= m;
        k ^= k >> r;
        k *= m;
        
        h ^= k;
        h *= m;
    }

    const uint8 * data2 = (const uint8*)data;
    switch(path.size() & 7) {
        case 7: h ^= uint64(data2[6]) << 48;
        case 6: h ^= uint64(data2[5]) << 40;
        case 5: h ^= uint64(data2[4]) << 32;
        case 4: h ^= uint64(data2[3]) << 24;
        case 3: h ^= uint64(data2[2]) << 16;
        case 2: h ^= uint64(data2[1]) << 8;
        case 1: h ^= uint64(data2[0]);
                h *= m;
    };
    
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return ".cache/" + std::string(std::to_string(h)) + ".stf";
}
