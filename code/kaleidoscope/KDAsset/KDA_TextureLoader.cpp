//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 14:58:00
//

#include "KDA_TextureLoader.h"

#include <stb_image.h>
#include <KernelCore/KC_FileSystem.h>

namespace KDA
{
    KGPU::TextureDesc Texture::ToTextureDesc()
    {
        KGPU::TextureDesc desc;
        desc.Width = Width;
        desc.Height = Height;
        desc.MipLevels = MipCount;
        desc.Format = Format;
        desc.Depth = 1;
        desc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage;

        return desc;
    }

    TextureType GetTextureType(const KC::String& extension)
    {
        if (extension == "jpg" || extension == "jpeg" || extension == "png")
            return TextureType::UncompressedImage;
        if (extension == "hdr" || extension == "exr")
            return TextureType::UncompressedFloatImage;
        return TextureType::Compressed;
    }

    Texture TextureLoader::LoadFromFile(const KC::String& path, bool srgb)
    {
        // Exists?
        if (!KC::FileSystem::Exists(path)) {
            KD_ERROR("%s doesn't exist!", path.c_str());
            return {};
        }

        // Get extension
        KC::String extension = KC::FileSystem::GetExtension(path);
        TextureType type = GetTextureType(extension);

        // Load!
        Texture result = {};
        result.Type = type;
        result.MipCount = 1;

        switch (type) {
            case TextureType::UncompressedImage: {
                int channels = 0;
                uint8* data = stbi_load(path.c_str(), &result.Width, &result.Height, &channels, STBI_rgb_alpha);
                KD_ASSERT_EQ(data, "Failed to load stbi data!");

                // Calculate number of mip levels
                uint maxDim = std::max(result.Width, result.Height);
                result.MipCount = floor(log2(std::max(result.Width, result.Height))) + 1;

                result.Bytes.resize(result.Width * result.Height * 4 * sizeof(uint8));
                result.Format = KGPU::TextureFormat::kR8G8B8A8_UNORM;
                memcpy(result.Bytes.data(), data, result.Bytes.size());
                stbi_image_free(data);
                return result;
            }
            case TextureType::UncompressedFloatImage: {
                int channels = 0;
                uint16* data = stbi_load_16(path.c_str(), &result.Width, &result.Height, &channels, STBI_rgb_alpha);
                KD_ASSERT_EQ(data, "Failed to load stbi data!");

                result.Bytes.resize(result.Width * result.Height * 4 * sizeof(uint16));
                result.Format = KGPU::TextureFormat::kR16G16B16A16_UNORM;
                memcpy(result.Bytes.data(), data, result.Bytes.size());
                stbi_image_free(data);
                return result;
            }
            case TextureType::Compressed: {
                KD_ERROR("Compressed textures are not supported!");
                return {};
            }
        }

        return {};
    }
}
