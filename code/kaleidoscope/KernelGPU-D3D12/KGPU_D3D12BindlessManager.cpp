//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:10:25
//

#include "KGPU_D3D12BindlessManager.h"
#include "KGPU_D3D12Device.h"
#include "KGPU_D3D12TextureView.h"
#include "KGPU_D3D12Texture.h"
#include "KGPU_D3D12Buffer.h"
#include "KGPU_D3D12BufferView.h"
#include "KGPU_D3D12Sampler.h"
#include "KGPU_D3D12TLAS.h"

namespace KGPU
{
    constexpr uint64 MAX_BINDLESS_RESOURCES = 400000;
    constexpr uint64 MAX_BINDLESS_SAMPLERS = 2048;
    constexpr uint64 MAX_RENDER_TARGETS = 2048;
    constexpr uint64 MAX_DEPTH_TARGETS = 2048;

    D3D12BindlessManager::D3D12BindlessManager(D3D12Device* device)
        : mParentDevice(device), mResourceAllocator(MAX_BINDLESS_RESOURCES), mSamplerAllocator(MAX_BINDLESS_SAMPLERS), mRTVAllocator(MAX_RENDER_TARGETS), mDSVAllocator(MAX_DEPTH_TARGETS)
    {
        // CBVSRVUAV
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = MAX_BINDLESS_RESOURCES;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        
            HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mResourceHeap));
            KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");
        
            mResourceIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
        }
        // SAMPLER
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            desc.NumDescriptors = MAX_BINDLESS_SAMPLERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        
            HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSamplerHeap));
            KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");
        
            mSamplerIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
        }
        // RTV
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = MAX_RENDER_TARGETS;
        
            HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mRTVHeap));
            KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");
        
            mRenderTargetIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
        }
        // UAV
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            desc.NumDescriptors = MAX_DEPTH_TARGETS;
        
            HRESULT result = device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDSVHeap));
            KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create descriptor heap!");
        
            mDepthStencilIncrement = device->GetDevice()->GetDescriptorHandleIncrementSize(desc.Type);
        }
    
        KD_WHATEVER("Initialized D3D12 descriptor heaps");
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
    
        uint availableIndex = mDSVAllocator.Allocate();
    
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Format = D3D12Texture::TranslateToDXGIFormat(desc.ViewFormat);
        if (desc.Dimension == TextureViewDimension::kTexture2D) {
            srvDesc.Texture2D.MipLevels = desc.ViewMip == VIEW_ALL_MIPS ? desc.Texture->GetDesc().MipLevels : 1;
            srvDesc.Texture2D.MostDetailedMip = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
        } else if (desc.Dimension == TextureViewDimension::kTextureCube) {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MipLevels = desc.ViewMip == VIEW_ALL_MIPS ? desc.Texture->GetDesc().MipLevels : 1;
            srvDesc.TextureCube.MostDetailedMip = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
        }
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        mParentDevice->GetDevice()->CreateShaderResourceView(resource, &srvDesc, alloc.CPU);
        return alloc;
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteTextureUAV(D3D12TextureView* srv)
    {
        ID3D12Resource* resource = static_cast<D3D12Texture*>(srv->GetDesc().Texture)->GetResource();
        auto desc = srv->GetDesc();
    
        uint availableIndex = mResourceAllocator.Allocate();
    
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Format = D3D12Texture::TranslateToDXGIFormat(desc.ViewFormat);
        if (desc.Dimension == TextureViewDimension::kTexture2D) {
            uavDesc.Texture2D.MipSlice = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
            uavDesc.Texture2D.PlaneSlice = 0;
        } else {
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            uavDesc.Texture2DArray.ArraySize = 6;
            uavDesc.Texture2DArray.FirstArraySlice = 0;
            uavDesc.Texture2DArray.MipSlice = desc.ViewMip == VIEW_ALL_MIPS ? 0 : desc.ViewMip;
        }
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        mParentDevice->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &uavDesc, alloc.CPU);
        return alloc;
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteBufferCBV(D3D12BufferView* cbv)
    {
        uint availableIndex = mResourceAllocator.Allocate();
    
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvd = {};
        cbvd.BufferLocation = cbv->GetDesc().Buffer->GetAddress();
        cbvd.SizeInBytes = cbv->GetDesc().Buffer->GetDesc().Size;
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        mParentDevice->GetDevice()->CreateConstantBufferView(&cbvd, alloc.CPU);
        return alloc;
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteBufferSRV(D3D12BufferView* cbv)
    {
        ID3D12Resource* resource = static_cast<D3D12Buffer*>(cbv->GetDesc().Buffer)->GetResource();
    
        uint availableIndex = mResourceAllocator.Allocate();
    
        D3D12_SHADER_RESOURCE_VIEW_DESC srv = {};
        srv.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_BUFFER;
        srv.Format = DXGI_FORMAT_UNKNOWN;
        srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        srv.Buffer.FirstElement = 0;
        srv.Buffer.NumElements = cbv->GetDesc().Buffer->GetDesc().Size / cbv->GetDesc().Buffer->GetDesc().Stride;
        srv.Buffer.StructureByteStride = cbv->GetDesc().Buffer->GetDesc().Stride;
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        mParentDevice->GetDevice()->CreateShaderResourceView(resource, &srv, alloc.CPU);
        return alloc;
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteBufferUAV(D3D12BufferView* cbv)
    {
        ID3D12Resource* resource = static_cast<D3D12Buffer*>(cbv->GetDesc().Buffer)->GetResource();
    
        uint availableIndex = mResourceAllocator.Allocate();
    
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavd = {};
        uavd.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavd.Format = DXGI_FORMAT_UNKNOWN;
        uavd.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        uavd.Buffer.FirstElement = 0;
        uavd.Buffer.NumElements = cbv->GetDesc().Buffer->GetDesc().Size / 4;
        uavd.Buffer.StructureByteStride = 4;
        uavd.Buffer.CounterOffsetInBytes = 0;
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        mParentDevice->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &uavd, alloc.CPU);
        return alloc;
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteAS(D3D12TLAS* as)
    {
        uint availableIndex = mResourceAllocator.Allocate();
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.RaytracingAccelerationStructure.Location = as->Address();
    
        mParentDevice->GetDevice()->CreateShaderResourceView(nullptr, &desc, alloc.CPU);
        return alloc;
    }
    
    void D3D12BindlessManager::FreeCBVSRVUAV(D3D12BindlessAlloc index)
    {
        mResourceAllocator.Free(index.Index);
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::FindFreeSpace()
    {
        uint availableIndex = mResourceAllocator.Allocate();
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mResourceHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.GPU = mResourceHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mResourceIncrement;
        alloc.GPU.ptr += availableIndex * mResourceIncrement;
    
        return alloc;
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteSampler(D3D12Sampler* sampler)
    {
        SamplerDesc desc = sampler->GetDesc();
    
        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.AddressU = TranslateD3DAddress(desc.Address);
        samplerDesc.AddressV = samplerDesc.AddressU;
        samplerDesc.AddressW = samplerDesc.AddressV;
        samplerDesc.Filter = TranslateD3DFilter(desc.Filter);
        samplerDesc.MaxAnisotropy = 16;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = desc.UseMips ? D3D12_FLOAT32_MAX : 1.0f;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.BorderColor[0] = 1.0f;
        samplerDesc.BorderColor[1] = 1.0f;
        samplerDesc.BorderColor[2] = 1.0f;
        samplerDesc.BorderColor[3] = 1.0f;
    
        uint availableIndex = mSamplerAllocator.Allocate();
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mSamplerHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mSamplerIncrement;
        alloc.GPU = mSamplerHeap->GetGPUDescriptorHandleForHeapStart();
        alloc.GPU.ptr += availableIndex * mSamplerIncrement;
    
        mParentDevice->GetDevice()->CreateSampler(&samplerDesc, alloc.CPU);
        return alloc;
    }
    
    void D3D12BindlessManager::FreeSampler(D3D12BindlessAlloc index)
    {
        mSamplerAllocator.Free(index.Index);
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteRTV(D3D12TextureView* rtv)
    {
        ID3D12Resource* resource = static_cast<D3D12Texture*>(rtv->GetDesc().Texture)->GetResource();
    
        uint availableIndex = mRTVAllocator.Allocate();
    
        D3D12_RENDER_TARGET_VIEW_DESC desc = {};
        desc.Format = D3D12Texture::TranslateToDXGIFormat(rtv->GetDesc().ViewFormat);
        if (rtv->GetDesc().Dimension == TextureViewDimension::kTexture2D) {
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.PlaneSlice = rtv->GetDesc().ArrayLayer;
        }
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mRTVHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mRenderTargetIncrement;
    
        mParentDevice->GetDevice()->CreateRenderTargetView(resource, &desc, alloc.CPU);
        return alloc;
    }
    
    void D3D12BindlessManager::FreeRTV(D3D12BindlessAlloc index)
    {
        mRTVAllocator.Free(index.Index);
    }
    
    D3D12BindlessAlloc D3D12BindlessManager::WriteDSV(D3D12TextureView* dsv)
    {
        ID3D12Resource* resource = static_cast<D3D12Texture*>(dsv->GetDesc().Texture)->GetResource();
    
        uint availableIndex = mDSVAllocator.Allocate();
    
        D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
        desc.Format = D3D12Texture::TranslateToDXGIFormat(dsv->GetDesc().ViewFormat);
        if (dsv->GetDesc().Dimension == TextureViewDimension::kTexture2D) {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        } else {
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            desc.Texture2DArray.ArraySize = 1;
            desc.Texture2DArray.FirstArraySlice = dsv->GetDesc().ArrayLayer;
            desc.Texture2DArray.MipSlice = 0;
        }
    
        D3D12BindlessAlloc alloc = {};
        alloc.Index = availableIndex;
        alloc.CPU = mDSVHeap->GetCPUDescriptorHandleForHeapStart();
        alloc.CPU.ptr += availableIndex * mDepthStencilIncrement;
    
        mParentDevice->GetDevice()->CreateDepthStencilView(resource, &desc, alloc.CPU);
        return alloc;
    }
    
    void D3D12BindlessManager::FreeDSV(D3D12BindlessAlloc index)
    {
        mDSVAllocator.Free(index.Index);
    }
    
    D3D12_TEXTURE_ADDRESS_MODE D3D12BindlessManager::TranslateD3DAddress(SamplerAddress address)
    {
        switch (address)
        {
            case SamplerAddress::kWrap:   return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case SamplerAddress::kMirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case SamplerAddress::kClamp:  return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case SamplerAddress::kBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            default:                         return D3D12_TEXTURE_ADDRESS_MODE_WRAP; // Fallback
        }
    }
    
    D3D12_FILTER D3D12BindlessManager::TranslateD3DFilter(SamplerFilter filter)
    {
        switch (filter)
        {
            case SamplerFilter::kLinear:  return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            case SamplerFilter::kNearest: return D3D12_FILTER_MIN_MAG_MIP_POINT;
            default:                         return D3D12_FILTER_MIN_MAG_MIP_LINEAR; // Fallback
        }
    }
}
