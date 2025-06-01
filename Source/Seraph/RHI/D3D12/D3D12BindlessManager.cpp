//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 16:14:52
//

#include "D3D12BindlessManager.h"
#include "D3D12Device.h"

constexpr uint64 MAX_BINDLESS_RESOURCES = 1000000;
constexpr uint64 MAX_BINDLESS_SAMPLERS = 2048;
constexpr uint64 MAX_RENDER_TARGETS = 2048;
constexpr uint64 MAX_DEPTH_TARGETS = 2048;

D3D12BindlessManager::D3D12BindlessManager(D3D12Device* device)
    : mParentDevice(device)
{
    // CBVSRVUAV
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = MAX_BINDLESS_RESOURCES;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mResourceHeap));
        ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");

        mResourceLUT.resize(MAX_BINDLESS_RESOURCES, false);
        mResourceIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
    }
    // SAMPLER
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.NumDescriptors = MAX_BINDLESS_SAMPLERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSamplerHeap));
        ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");

        mSamplerLUT.resize(MAX_BINDLESS_SAMPLERS, false);
        mSamplerIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
    }
    // RTV
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = MAX_RENDER_TARGETS;

        HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mRTVHeap));
        ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");

        mRTVLUT.resize(MAX_RENDER_TARGETS, false);
        mRenderTargetIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
    }
    // UAV
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.NumDescriptors = MAX_DEPTH_TARGETS;

        HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDSVHeap));
        ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");

        mDSVLUT.resize(MAX_DEPTH_TARGETS, false);
        mDepthStencilIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
    }

    SERAPH_WHATEVER("Initialized D3D12 descriptor heaps");
}

D3D12BindlessManager::~D3D12BindlessManager()
{
    if (mResourceHeap) mResourceHeap->Release();
    if (mSamplerHeap) mSamplerHeap->Release();
    if (mDSVHeap) mDSVHeap->Release();
    if (mRTVHeap) mRTVHeap->Release();
}

D3D12BindlessAlloc D3D12BindlessManager::WriteTextureSRV(D3D12TextureView* srv)
{
    return {};
}

D3D12BindlessAlloc D3D12BindlessManager::WriteTextureUAV(D3D12TextureView* srv)
{
    return {};
}

D3D12BindlessAlloc D3D12BindlessManager::WriteBufferCBV(D3D12BufferView* cbv)
{
    return {};
}

D3D12BindlessAlloc D3D12BindlessManager::WriteBufferSRV(D3D12BufferView* cbv)
{
    return {};
}

D3D12BindlessAlloc D3D12BindlessManager::WriteBufferUAV(D3D12BufferView* cbv)
{
    return {};
}

D3D12BindlessAlloc D3D12BindlessManager::WriteAS(D3D12TLAS* as)
{
    return {};
}

void D3D12BindlessManager::FreeCBVSRVUAV(D3D12BindlessAlloc index)
{

}

D3D12BindlessAlloc D3D12BindlessManager::WriteSampler(D3D12Sampler* sampler)
{
    return {};
}

void D3D12BindlessManager::FreeSampler(D3D12BindlessAlloc index)
{

}

D3D12BindlessAlloc D3D12BindlessManager::WriteRTV(D3D12TextureView* rtv)
{
    return {};
}

void D3D12BindlessManager::FreeRTV(D3D12BindlessAlloc index)
{
    
}

D3D12BindlessAlloc D3D12BindlessManager::WriteDSV(D3D12TextureView* dsv)
{
    return {};
}

void D3D12BindlessManager::FreeDSV(D3D12BindlessAlloc index)
{

}
