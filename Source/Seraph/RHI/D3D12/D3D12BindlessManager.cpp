//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 16:14:52
//

#include "D3D12BindlessManager.h"
#include "D3D12Device.h"
#include "D3D12TextureView.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12BufferView.h"

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
    ID3D12Resource* resource = static_cast<D3D12Texture*>(srv->GetDesc().Texture)->GetResource();
    auto desc = srv->GetDesc();

    uint availableIndex = 0;
    for (uint i = 0; i < mResourceLUT.size(); i++) {
        if (mResourceLUT[i] == false) {
            mResourceLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Format = D3D12Texture::TranslateToDXGIFormat(desc.ViewFormat);
    if (desc.Dimension == RHITextureViewDimension::kTexture2D) {
        srvDesc.Texture2D.MipLevels = desc.ViewMip == VIEW_ALL_MIPS ? desc.Texture->GetDesc().MipLevels : 1;
        srvDesc.Texture2D.MostDetailedMip = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
    } else if (desc.Dimension == RHITextureViewDimension::kTextureCube) {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.TextureCube.MipLevels = desc.ViewMip == VIEW_ALL_MIPS ? desc.Texture->GetDesc().MipLevels : 1;
        srvDesc.TextureCube.MostDetailedMip = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;
    alloc.GPU = gpu;

    mParentDevice->GetDevice()->CreateShaderResourceView(resource, &srvDesc, alloc.CPU);
    return alloc;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteTextureUAV(D3D12TextureView* srv)
{
    ID3D12Resource* resource = static_cast<D3D12Texture*>(srv->GetDesc().Texture)->GetResource();
    auto desc = srv->GetDesc();

    uint availableIndex = 0;
    for (uint i = 0; i < mResourceLUT.size(); i++) {
        if (mResourceLUT[i] == false) {
            mResourceLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format = D3D12Texture::TranslateToDXGIFormat(desc.ViewFormat);
    if (desc.Dimension == RHITextureViewDimension::kTexture2D) {
        uavDesc.Texture2D.MipSlice = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
        uavDesc.Texture2D.PlaneSlice = 0;
    } else {
        uavDesc.Texture2DArray.ArraySize = 6;
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.MipSlice = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;
    alloc.GPU = gpu;

    mParentDevice->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &uavDesc, alloc.CPU);
    return alloc;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteBufferCBV(D3D12BufferView* cbv)
{
    uint availableIndex = 0;
    for (uint i = 0; i < mResourceLUT.size(); i++) {
        if (mResourceLUT[i] == false) {
            mResourceLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd = {};
    cbvd.BufferLocation = cbv->GetDesc().Buffer->GetAddress();
    cbvd.SizeInBytes = cbv->GetDesc().Buffer->GetDesc().Size;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;
    alloc.GPU = gpu;

    mParentDevice->GetDevice()->CreateConstantBufferView(&cbvd, alloc.CPU);
    return alloc;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteBufferSRV(D3D12BufferView* cbv)
{
    ID3D12Resource* resource = static_cast<D3D12Buffer*>(cbv->GetDesc().Buffer)->GetResource();

    uint availableIndex = 0;
    for (uint i = 0; i < mResourceLUT.size(); i++) {
        if (mResourceLUT[i] == false) {
            mResourceLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
    srv.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
    srv.Format = DXGI_FORMAT_UNKNOWN;
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
    srv.Buffer.FirstElement = 0;
    srv.Buffer.NumElements = cbv->GetDesc().Buffer->GetDesc().Size / cbv->GetDesc().Buffer->GetDesc().Stride;
    srv.Buffer.StructureByteStride = cbv->GetDesc().Buffer->GetDesc().Stride;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;
    alloc.GPU = gpu;

    mParentDevice->GetDevice()->CreateShaderResourceView(resource, &srv, alloc.CPU);
    return alloc;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteBufferUAV(D3D12BufferView* cbv)
{
    ID3D12Resource* resource = static_cast<D3D12Buffer*>(cbv->GetDesc().Buffer)->GetResource();

    uint availableIndex = 0;
    for (uint i = 0; i < mResourceLUT.size(); i++) {
        if (mResourceLUT[i] == false) {
            mResourceLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavd = {};
    uavd.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavd.Format = DXGI_FORMAT_UNKNOWN;
    uavd.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
    uavd.Buffer.FirstElement = 0;
    uavd.Buffer.NumElements = cbv->GetDesc().Buffer->GetDesc().Size / 4;
    uavd.Buffer.StructureByteStride = 4;
    uavd.Buffer.CounterOffsetInBytes = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12_GPU_DESCRIPTOR_HANDLE gpu = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
    gpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;
    alloc.GPU = gpu;

    mParentDevice->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &uavd, alloc.CPU);
    return alloc;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteAS(D3D12TLAS* as)
{
    // TODO
    return {};
}

void D3D12BindlessManager::FreeCBVSRVUAV(D3D12BindlessAlloc index)
{
    mResourceLUT[index.Index] = false;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteSampler(D3D12Sampler* sampler)
{
    // TODO
    return {};
}

void D3D12BindlessManager::FreeSampler(D3D12BindlessAlloc index)
{
    mSamplerLUT[index.Index] = false;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteRTV(D3D12TextureView* rtv)
{
    ID3D12Resource* resource = static_cast<D3D12Texture*>(rtv->GetDesc().Texture)->GetResource();

    uint availableIndex = 0;
    for (uint i = 0; i < mRTVLUT.size(); i++) {
        if (mRTVLUT[i] == false) {
            mRTVLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_RENDER_TARGET_VIEW_DESC desc = {};
    desc.Format = D3D12Texture::TranslateToDXGIFormat(rtv->GetDesc().ViewFormat);
    if (rtv->GetDesc().Dimension == RHITextureViewDimension::kTexture2D) {
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.PlaneSlice = rtv->GetDesc().ArrayLayer;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;

    mParentDevice->GetDevice()->CreateRenderTargetView(resource, &desc, alloc.CPU);
    return alloc;
}

void D3D12BindlessManager::FreeRTV(D3D12BindlessAlloc index)
{
    mRTVLUT[index.Index] = false;
}

D3D12BindlessAlloc D3D12BindlessManager::WriteDSV(D3D12TextureView* dsv)
{
    ID3D12Resource* resource = static_cast<D3D12Texture*>(dsv->GetDesc().Texture)->GetResource();

    uint availableIndex = 0;
    for (uint i = 0; i < mDSVLUT.size(); i++) {
        if (mDSVLUT[i] == false) {
            mDSVLUT[i] = true;
            availableIndex = i;
            break;
        }
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.Format = D3D12Texture::TranslateToDXGIFormat(dsv->GetDesc().ViewFormat);
    if (dsv->GetDesc().Dimension == RHITextureViewDimension::kTexture2D) {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    } else {
        desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        desc.Texture2DArray.ArraySize = 1;
        desc.Texture2DArray.FirstArraySlice = dsv->GetDesc().ArrayLayer;
        desc.Texture2DArray.MipSlice = 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE cpu = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += availableIndex * mResourceIncrement;

    D3D12BindlessAlloc alloc = {};
    alloc.Index = availableIndex;
    alloc.CPU = cpu;

    mParentDevice->GetDevice()->CreateDepthStencilView(resource, &desc, alloc.CPU);
    return alloc;
}

void D3D12BindlessManager::FreeDSV(D3D12BindlessAlloc index)
{
    mDSVLUT[index.Index] = false;
}
