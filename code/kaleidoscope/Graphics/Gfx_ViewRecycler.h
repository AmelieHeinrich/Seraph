//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 13:21:13
//

#pragma once

#include <KernelCore/KC_HashMap.h>
#include "Gfx_ResourceManager.h"

namespace Gfx {
    struct TextureViewKey
    {
        const KGPU::ITexture* Texture;
        KGPU::TextureViewDesc Desc = {};

        bool operator==(const TextureViewKey& other) const {
            return Texture == other.Texture && Desc == other.Desc;
        }
    };

    struct BufferViewKey
    {
        const KGPU::IBuffer* Buffer;
        KGPU::BufferViewDesc Desc = {};

        bool operator==(const BufferViewKey& other) const {
            return Buffer == other.Buffer && Desc == other.Desc;
        }
    };
}

namespace std {
    template <>
    struct hash<KGPU::TextureViewDesc> {
        size_t operator()(const KGPU::TextureViewDesc& desc) const {
            size_t h1 = hash<const void*>{}(desc.Texture);
            size_t h2 = hash<int>{}(static_cast<int>(desc.Type));
            size_t h3 = hash<int>{}(static_cast<int>(desc.Dimension));
            size_t h4 = hash<int>{}(static_cast<int>(desc.ViewFormat));
            size_t h5 = hash<uint64_t>{}(desc.ViewMip);
            size_t h6 = hash<uint64_t>{}(desc.ArrayLayer);

            size_t result = h1;
            result ^= (h2 << 1);
            result ^= (h3 << 2);
            result ^= (h4 << 3);
            result ^= (h5 << 4);
            result ^= (h6 << 5);
            return result;
        }
    };

    template <>
    struct hash<KGPU::BufferViewDesc> {
        size_t operator()(const KGPU::BufferViewDesc& desc) const {
            size_t h1 = hash<const void*>{}(desc.Buffer);
            size_t h2 = hash<int>{}(static_cast<int>(desc.Type));
            return h1 ^ (h2 << 1);
        }
    };

    template <>
    struct hash<Gfx::TextureViewKey> {
        size_t operator()(const Gfx::TextureViewKey& key) const {
            size_t h1 = hash<const void*>{}(key.Texture);
            size_t h2 = hash<KGPU::TextureViewDesc>{}(key.Desc);
            return h1 ^ (h2 << 1);
        }
    };

    template <>
    struct hash<Gfx::BufferViewKey> {
        size_t operator()(const Gfx::BufferViewKey& key) const {
            size_t h1 = hash<const void*>{}(key.Buffer);
            size_t h2 = hash<KGPU::BufferViewDesc>{}(key.Desc);
            return h1 ^ (h2 << 1);
        }
    };
}

namespace Gfx
{
    class ViewRecycler
    {
    public:
        static void Clean();

        static KGPU::ITextureView* GetSRV(KGPU::ITexture* texture);
        static KGPU::ITextureView* GetRTV(KGPU::ITexture* texture);
        static KGPU::ITextureView* GetUAV(KGPU::ITexture* texture);
        static KGPU::ITextureView* GetDSV(KGPU::ITexture* texture);
    
        static KGPU::IBufferView* GetSRV(KGPU::IBuffer* buffer);
        static KGPU::IBufferView* GetUAV(KGPU::IBuffer* buffer);
        static KGPU::IBufferView* GetCBV(KGPU::IBuffer* buffer);

        static KGPU::ITextureView* GetTextureView(const KGPU::TextureViewDesc& desc);
        static KGPU::IBufferView* GetBufferView(const KGPU::BufferViewDesc& desc);
    private:
        static struct Data {
            KC::HashMap<TextureViewKey, KGPU::ITextureView*> TextureViews;
            KC::HashMap<BufferViewKey, KGPU::IBufferView*> BufferViews;
        } sData;
    };
}
