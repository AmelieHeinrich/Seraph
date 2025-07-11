//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:59:55
//

#pragma once

#include <RHI/Buffer.h>

class DummyDevice;

class DummyBuffer : public IRHIBuffer
{
public:
    DummyBuffer(DummyDevice* device, RHIBufferDesc desc);
    ~DummyBuffer();

    void SetName(const String& name) override;

    void* Map() override;
    void Unmap() override;

    uint64 GetAddress() override;

private:
    DummyDevice* mParentDevice;
    void* mMappedMemory;
};
