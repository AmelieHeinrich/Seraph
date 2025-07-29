//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:50:58
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelCore/KC_FreeList.h>

#include <Agility/d3d12.h>

#include <KernelGPU/KGPU_Sampler.h>

namespace KGPU
{
    class D3D12Device;
    class D3D12TextureView;
    class D3D12Sampler;
    class D3D12TLAS;
    class D3D12BufferView;

    struct D3D12BindlessAlloc
    {
        uint Index;
        D3D12_CPU_DESCRIPTOR_HANDLE CPU;
        D3D12_GPU_DESCRIPTOR_HANDLE GPU;
    };

    class D3D12BindlessManager
    {
    public:
        D3D12BindlessManager(D3D12Device* device);
        ~D3D12BindlessManager();

        // ResourceDescriptorHeap[]
        D3D12BindlessAlloc WriteTextureSRV(D3D12TextureView* srv);
        D3D12BindlessAlloc WriteTextureUAV(D3D12TextureView* srv);
        D3D12BindlessAlloc WriteBufferCBV(D3D12BufferView* cbv);
        D3D12BindlessAlloc WriteBufferSRV(D3D12BufferView* cbv);
        D3D12BindlessAlloc WriteBufferUAV(D3D12BufferView* cbv);
        D3D12BindlessAlloc WriteAS(D3D12TLAS* as);
        void FreeCBVSRVUAV(D3D12BindlessAlloc index);

        D3D12BindlessAlloc FindFreeSpace();

        // SamplerDescriptorHeap[]
        D3D12BindlessAlloc WriteSampler(D3D12Sampler* sampler);
        void FreeSampler(D3D12BindlessAlloc index);

        // RTVHeap[]
        D3D12BindlessAlloc WriteRTV(D3D12TextureView* rtv);
        void FreeRTV(D3D12BindlessAlloc index);

        // DSVHeap[]
        D3D12BindlessAlloc WriteDSV(D3D12TextureView* dsv);
        void FreeDSV(D3D12BindlessAlloc index);

    public:
        ID3D12DescriptorHeap* GetResourceHeap() { return mResourceHeap; }
        ID3D12DescriptorHeap* GetSamplerHeap() { return mSamplerHeap; }

    private:
        D3D12_TEXTURE_ADDRESS_MODE TranslateD3DAddress(SamplerAddress address);
        D3D12_FILTER TranslateD3DFilter(SamplerFilter filter);

    private:
        D3D12Device* mParentDevice;

        KC::FreeList mResourceAllocator;
        ID3D12DescriptorHeap* mResourceHeap;
        uint64 mResourceIncrement;

        KC::FreeList mSamplerAllocator;
        ID3D12DescriptorHeap* mSamplerHeap;
        uint64 mSamplerIncrement;

        KC::FreeList mRTVAllocator;
        ID3D12DescriptorHeap* mRTVHeap;
        uint64 mRenderTargetIncrement;

        KC::FreeList mDSVAllocator;
        ID3D12DescriptorHeap* mDSVHeap;
        uint64 mDepthStencilIncrement;
    };
}
