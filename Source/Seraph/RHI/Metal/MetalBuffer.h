//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:59:55
//

#pragma once

#include <RHI/Buffer.h>
#include <MetalCPP/Metal/Metal.hpp>

class MetalDevice;

class MetalBuffer : public IRHIBuffer
{
public:
    MetalBuffer(MetalDevice* device, RHIBufferDesc desc);
    ~MetalBuffer();

    void SetName(const std::string& name) override;

    void* Map() override;
    void Unmap() override;

    uint64 GetAddress() override;

public:
    MTL::Buffer* GetBuffer() { return mBuffer; }

private:
    MetalDevice* mParentDevice;
    
    MTL::Buffer* mBuffer;
    NS::String* mLabel;
};
