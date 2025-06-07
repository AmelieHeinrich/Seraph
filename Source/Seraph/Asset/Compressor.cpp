//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 19:48:16
//

#include "Compressor.h"
#include "Texture.h"

#include <filesystem>
#include <Core/FileSystem.h>

class NVTTWriter : public nvtt::OutputHandler
{
public:
    NVTTWriter(TextureHeader header, const String& path) {
        mFile = fopen(path.c_str(), "wb+");
        fwrite(&header, sizeof(header), 1, mFile);
    }

    ~NVTTWriter() {
        fclose(mFile);
    }

    virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override {}
    virtual void endImage() override {}

    virtual bool writeData(const void* data, int size) override {
        fwrite(data, size, 1, mFile);
        return true;
    }
private:
    FILE* mFile;
};

void Compressor::RecurseFolder(const String& path)
{
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path)) {
        String entryPath = dirEntry.path().string();
        std::replace(entryPath.begin(), entryPath.end(), '\\', '/');
    
        if (dirEntry.path().extension() == ".png" || dirEntry.path().extension() == ".jpg") {
            CompressTexture(entryPath);
        }
    }
}

void Compressor::CompressTexture(const String& path)
{
    if (FileSystem::Exists(ToCachedPath(path))) {
        SERAPH_WHATEVER("Skipping %s", path.c_str());
        return;
    }

    nvtt::Surface image;
    if (!image.load(path.c_str())) {
        SERAPH_ERROR("Failed to load NVTT image at path %s", path.c_str());
        return;
    }

    int width = image.width();
    int height = image.height();
    int mipCount = image.countMipmaps();
    int finalMipCount = glm::max(1, mipCount - 2); // Skip 2x2 and 1x1

    TextureHeader header;
    header.Width = width;
    header.Height = height;
    header.Mips = mipCount;
    header.Format = RHITextureFormat::kBC7_UNORM;

    NVTTWriter writer(header, ToCachedPath(path));
    
    nvtt::OutputOptions options;
    options.setOutputHandler(&writer);

    nvtt::CompressionOptions compressionOptions;
    compressionOptions.setFormat(nvtt::Format_BC7);

    for (int i = 0; i < finalMipCount; i++) {
        if (!mContext.compress(image, 0, i, compressionOptions, options)) {
            SERAPH_ERROR("Failed to compress texture!");
        }

        // Prepare the next mip:
        image.toLinearFromSrgb();
        image.premultiplyAlpha();

        image.buildNextMipmap(nvtt::MipmapFilter_Box);

        image.demultiplyAlpha();
        image.toSrgb();
    }
    SERAPH_WHATEVER("Compressed %s", path.c_str());
}

String Compressor::ToCachedPath(const String& path)
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
    return ".cache/" + String(std::to_string(h)) + ".stf";
}
