//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:59:55
//

#pragma once

#include <RHI/Buffer.h>

#include <Agility/d3d12.h>
#include <D3D12MA/D3D12MemAlloc.h>

class D3D12Device;

class D3D12Buffer : public IRHIBuffer
{
public:
    D3D12Buffer(D3D12Device* device, RHIBufferDesc desc);
    ~D3D12Buffer();

    void SetName(const StringView& name) override;

    void* Map() override;
    void Unmap() override;

    uint64 GetAddress() override;

public:
    D3D12MA::Allocation* GetAllocation() { return mAllocation; }
    ID3D12Resource* GetResource() { return mResource; }

private:
    ID3D12Resource* mResource;
    D3D12MA::Allocation* mAllocation;
};
