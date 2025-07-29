//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:26:18
//

#pragma once

#include <KernelGPU/KGPU_Buffer.h>

#include <Agility/d3d12.h>

namespace KGPU
{
    class D3D12Device;

    class D3D12Buffer : public IBuffer
    {
    public:
        D3D12Buffer(D3D12Device* device, BufferDesc desc);
        ~D3D12Buffer();

        void SetName(const KC::String& name) override;

        void* Map() override;
        void Unmap(uint start = 0, uint end = 0) override;

        uint64 GetAddress() override;

    public:
        ID3D12Resource* GetResource() { return mResource; }

    private:
        D3D12Device* mParentDevice;

        ID3D12Resource* mResource;
    };
}
