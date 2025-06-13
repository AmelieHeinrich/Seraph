//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:59:55
//

#pragma once

#include <RHI/Buffer.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12Buffer : public IRHIBuffer
{
public:
    D3D12Buffer(D3D12Device* device, RHIBufferDesc desc);
    ~D3D12Buffer();

    void SetName(const String& name) override;

    void* Map() override;
    void Unmap() override;

    uint64 GetAddress() override;

public:
    ID3D12Resource* GetResource() { return mResource; }

private:
    D3D12Device* mParentDevice;

    ID3D12Resource* mResource;
};
