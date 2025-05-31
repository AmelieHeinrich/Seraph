//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 21:52:29
//

#pragma once

#include <Core/Context.h>

enum class RHIBufferUsage
{
    kVertex = BIT(0),
    kIndex = BIT(1),
    kConstant = BIT(2),
    kShaderRead = BIT(3),
    kShaderWrite = BIT(4),
    kStaging = BIT(5),
    kReadback = BIT(6),
    kAccelerationStructure = BIT(7),
    kShaderBindingTable = BIT(8)
};
ENUM_CLASS_FLAGS(RHIBufferUsage);

struct RHIBufferDesc
{
    uint64 Size;
    uint64 Stride = 0;
    RHIBufferUsage Usage;

    RHIBufferDesc() = default;
    RHIBufferDesc(uint64 size, uint64 stride, RHIBufferUsage usage)
        : Size(size), Stride(stride), Usage(usage) {}
};

class IRHIBuffer
{
public:
    ~IRHIBuffer() = default;

    virtual void SetName(const StringView& name) = 0;

    RHIBufferDesc GetDesc() const { return mDesc; }

    virtual void* Map() = 0;
    virtual void Unmap() = 0;
    virtual uint64 GetAddress() = 0;
protected:
    RHIBufferDesc mDesc;
};
