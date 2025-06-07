//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:00:38
//

#pragma once

#include "RendererResource.h"

struct TextureViewKey
{
    const IRHITexture* Texture;
    RHITextureViewDesc Desc;

    bool operator==(const TextureViewKey& other) const {
        return Texture == other.Texture && memcmp(&Desc, &other.Desc, sizeof(Desc)) == 0;
    }
};

struct BufferViewKey
{
    const IRHIBuffer* Buffer;
    RHIBufferViewDesc Desc;

    bool operator==(const BufferViewKey& other) const {
        return Buffer == other.Buffer && memcmp(&Desc, &other.Desc, sizeof(Desc)) == 0;
    }
};

namespace std {
    template <>
    struct hash<TextureViewKey> {
        std::size_t operator()(const TextureViewKey& key) const {
            std::size_t h1 = std::hash<const void*>{}(key.Texture);
            std::size_t h2 = std::hash<std::string_view>{}(
                std::string_view(reinterpret_cast<const char*>(&key.Desc), sizeof(key.Desc))
            );
            return h1 ^ (h2 << 1); // Combine hashes
        }
    };
}

namespace std {
    template <>
    struct hash<BufferViewKey> {
        std::size_t operator()(const BufferViewKey& key) const {
            std::size_t h1 = std::hash<const void*>{}(key.Buffer);
            std::size_t h2 = std::hash<std::string_view>{}(
                std::string_view(reinterpret_cast<const char*>(&key.Desc), sizeof(key.Desc))
            );
            return h1 ^ (h2 << 1); // Combine hashes
        }
    };
}

class RendererViewRecycler
{
public:
    static void Initialize(IRHIDevice* device);
    static void Shutdown();

    static IRHITextureView* GetSRV(IRHITexture* texture);
    static IRHITextureView* GetUAV(IRHITexture* texture);
    static IRHITextureView* GetDSV(IRHITexture* texture);
    static IRHITextureView* GetRTV(IRHITexture* texture);

    static IRHIBufferView* GetSRV(IRHIBuffer* buffer);
    static IRHIBufferView* GetUAV(IRHIBuffer* buffer);
    static IRHIBufferView* GetCBV(IRHIBuffer* buffer);

    static IRHITextureView* GetTextureView(const RHITextureViewDesc& desc);
    static IRHIBufferView* GetBufferView(const RHIBufferViewDesc& desc);
private:
    static struct Data {
        IRHIDevice* ParentDevice;
        UnorderedMap<TextureViewKey, IRHITextureView*> TextureViews;
        UnorderedMap<BufferViewKey, IRHIBufferView*> BufferViews;
    } sData;
};
