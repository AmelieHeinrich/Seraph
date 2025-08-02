//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:58:44
//

#include "KGPU_D3D12Texture.h"
#include "KGPU_D3D12Device.h"

namespace KGPU
{
    D3D12Texture::D3D12Texture(TextureDesc desc)
    {
        mDesc = desc;
        for (int i = 0; i < desc.MipLevels; i++) {
            mLayouts.push_back(KGPU::ResourceLayout::kUndefined);
        }
    }
    
    D3D12Texture::D3D12Texture(D3D12Device* device, TextureDesc desc)
    {
        mDesc = desc;
        for (int i = 0; i < desc.MipLevels; i++) {
            mLayouts.push_back(KGPU::ResourceLayout::kUndefined);
        }
        
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
        if (Any(desc.Usage & TextureUsage::kRenderTarget)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        if (Any(desc.Usage & TextureUsage::kStorage)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        if (Any(desc.Usage & TextureUsage::kDepthTarget)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    
        D3D12_HEAP_PROPERTIES allocationDesc = {};
        allocationDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
    
        HRESULT hr = device->GetDevice()->CreateCommittedResource(&allocationDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mResource));
        KD_ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 texture!");
    
        KD_WHATEVER("Created D3D12 texture");
    }
    
    D3D12Texture::~D3D12Texture()
    {
        if (mResource && !mDesc.Reserved) mResource->Release();
    }
    
    void D3D12Texture::SetName(const KC::String& name)
    {
        mResource->SetName(MULTIBYTE_TO_UNICODE(name.c_str()));
    }
    
    DXGI_FORMAT D3D12Texture::TranslateToDXGIFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::kB8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case TextureFormat::kD32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
            case TextureFormat::kR8G8B8A8_sRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case TextureFormat::kR8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::kR16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::kR16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case TextureFormat::kR32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
            case TextureFormat::kBC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
            case TextureFormat::kBC7_sRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
            case TextureFormat::kR16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::kR16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
            case TextureFormat::kR8_UINT: return DXGI_FORMAT_R8_UINT;
            case TextureFormat::kR16_UINT: return DXGI_FORMAT_R16_UINT;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
}
