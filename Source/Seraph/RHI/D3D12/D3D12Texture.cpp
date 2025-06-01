//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:50:54
//

#include "D3D12Texture.h"
#include "D3D12Device.h"

#include <Core/String.h>

D3D12Texture::D3D12Texture(D3D12Device* device, RHITextureDesc desc)
{
    mDesc = desc;
    
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Width = desc.Width;
    resourceDesc.Height = desc.Height;
    resourceDesc.DepthOrArraySize = desc.Depth;
    resourceDesc.Format = TranslateToDXGIFormat(desc.Format);
    resourceDesc.MipLevels = desc.MipLevels;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    if (Any(desc.Usage & RHITextureUsage::kRenderTarget)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if (Any(desc.Usage & RHITextureUsage::kStorage)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (Any(desc.Usage & RHITextureUsage::kDepthTarget)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr = device->GetAllocator()->CreateResource(&allocationDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &mAllocation, IID_PPV_ARGS(&mResource));
    ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 texture!");
}

D3D12Texture::~D3D12Texture()
{
    if (mAllocation && !mDesc.Reserved) mAllocation->Release();
}

void D3D12Texture::SetName(const StringView& name)
{
    mAllocation->SetName(MULTIBYTE_TO_UNICODE(name.data()));
    mResource->SetName(MULTIBYTE_TO_UNICODE(name.data()));
}

DXGI_FORMAT D3D12Texture::TranslateToDXGIFormat(RHITextureFormat format)
{
    switch (format)
    {
        case RHITextureFormat::kB8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case RHITextureFormat::kD32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
        case RHITextureFormat::kR8G8B8A8_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case RHITextureFormat::kR8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
    return DXGI_FORMAT_UNKNOWN;
}
