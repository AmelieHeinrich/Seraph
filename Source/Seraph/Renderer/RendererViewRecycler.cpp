//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:11:10
//

#include "RendererViewRecycler.h"

RendererViewRecycler::Data RendererViewRecycler::sData;

void RendererViewRecycler::Initialize(IRHIDevice* device)
{
    sData.ParentDevice;
}

void RendererViewRecycler::Shutdown()
{
    for (auto& bufferView : sData.BufferViews) {
        delete bufferView.second;
    }
    for (auto& textureView : sData.TextureViews) {
        delete textureView.second;
    }
    sData.BufferViews.clear();
    sData.TextureViews.clear();
}

IRHITextureView* RendererViewRecycler::GetSRV(IRHITexture* texture)
{
    return GetTextureView(RHITextureViewDesc(texture, RHITextureViewType::kShaderRead));
}

IRHITextureView* RendererViewRecycler::GetUAV(IRHITexture* texture)
{
    return GetTextureView(RHITextureViewDesc(texture, RHITextureViewType::kShaderWrite));
}

IRHITextureView* RendererViewRecycler::GetDSV(IRHITexture* texture)
{
    return GetTextureView(RHITextureViewDesc(texture, RHITextureViewType::kDepthTarget));
}

IRHITextureView* RendererViewRecycler::GetRTV(IRHITexture* texture)
{
    return GetTextureView(RHITextureViewDesc(texture, RHITextureViewType::kRenderTarget));
}

IRHIBufferView* RendererViewRecycler::GetSRV(IRHIBuffer* buffer)
{
    return GetBufferView(RHIBufferViewDesc(buffer, RHIBufferViewType::kStructured));
}

IRHIBufferView* RendererViewRecycler::GetUAV(IRHIBuffer* buffer)
{
    return GetBufferView(RHIBufferViewDesc(buffer, RHIBufferViewType::kStorage));
}

IRHIBufferView* RendererViewRecycler::GetCBV(IRHIBuffer* buffer)
{
    return GetBufferView(RHIBufferViewDesc(buffer, RHIBufferViewType::kConstant));
}

IRHITextureView* RendererViewRecycler::GetTextureView(const RHITextureViewDesc& desc)
{
    TextureViewKey key{desc.Texture, desc};
    auto it = sData.TextureViews.find(key);
    if (it != sData.TextureViews.end())
        return it->second;

    auto view = sData.ParentDevice->CreateTextureView(desc);
    sData.TextureViews.emplace(std::move(key), std::move(view));
    return view;
}

IRHIBufferView* RendererViewRecycler::GetBufferView(const RHIBufferViewDesc& desc)
{
    BufferViewKey key{desc.Buffer, desc};
    auto it = sData.BufferViews.find(key);
    if (it != sData.BufferViews.end())
        return it->second;

    auto view = sData.ParentDevice->CreateBufferView(desc);
    sData.BufferViews.emplace(std::move(key), std::move(view));
    return view;
}
