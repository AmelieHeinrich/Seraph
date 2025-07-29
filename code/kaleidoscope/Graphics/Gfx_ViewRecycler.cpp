//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 13:28:23
//

#include "Gfx_ViewRecycler.h"
#include "Gfx_Manager.h"

namespace Gfx
{
    ViewRecycler::Data ViewRecycler::sData;

    void ViewRecycler::Clean()
    {
        for (auto& bufferView : sData.BufferViews) {
            KC_DELETE(bufferView.second);
        }
        for (auto& textureView : sData.TextureViews) {
            KC_DELETE(textureView.second);
        }
        sData.BufferViews.clear();
        sData.TextureViews.clear();
    }

    KGPU::ITextureView* ViewRecycler::GetSRV(KGPU::ITexture* texture)
    {
        return GetTextureView(KGPU::TextureViewDesc(texture, KGPU::TextureViewType::kShaderRead));
    }

    KGPU::ITextureView* ViewRecycler::GetRTV(KGPU::ITexture* texture)
    {
        return GetTextureView(KGPU::TextureViewDesc(texture, KGPU::TextureViewType::kRenderTarget));
    }

    KGPU::ITextureView* ViewRecycler::GetUAV(KGPU::ITexture* texture)
    {
        return GetTextureView(KGPU::TextureViewDesc(texture, KGPU::TextureViewType::kShaderWrite));
    }

    KGPU::ITextureView* ViewRecycler::GetDSV(KGPU::ITexture* texture)
    {
        return GetTextureView(KGPU::TextureViewDesc(texture, KGPU::TextureViewType::kDepthTarget));
    }

    KGPU::IBufferView* ViewRecycler::GetSRV(KGPU::IBuffer* buffer)
    {
        return GetBufferView(KGPU::BufferViewDesc(buffer, KGPU::BufferViewType::kStructured));
    }

    KGPU::IBufferView* ViewRecycler::GetUAV(KGPU::IBuffer* buffer)
    {
        return GetBufferView(KGPU::BufferViewDesc(buffer, KGPU::BufferViewType::kStorage));
    }

    KGPU::IBufferView* ViewRecycler::GetCBV(KGPU::IBuffer* buffer)
    {
        return GetBufferView(KGPU::BufferViewDesc(buffer, KGPU::BufferViewType::kConstant));
    }

    KGPU::ITextureView* ViewRecycler::GetTextureView(const KGPU::TextureViewDesc& desc)
    {
        TextureViewKey key{desc.Texture, desc};
        auto it = sData.TextureViews.find(key);
        if (it != sData.TextureViews.end())
            return it->second;

        auto view = Manager::GetDevice()->CreateTextureView(desc);
        sData.TextureViews.emplace(std::move(key), std::move(view));
        return view;
    }

    KGPU::IBufferView* ViewRecycler::GetBufferView(const KGPU::BufferViewDesc& desc)
    {
        BufferViewKey key{desc.Buffer, desc};
        auto it = sData.BufferViews.find(key);
        if (it != sData.BufferViews.end())
            return it->second;

        auto view = Manager::GetDevice()->CreateBufferView(desc);
        sData.BufferViews.emplace(std::move(key), std::move(view));
        return view;
    }
}
