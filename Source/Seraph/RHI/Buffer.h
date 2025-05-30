//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-30 21:52:29
//

#pragma once

#include <Core/Context.h>

enum class RHIBufferUsage
{
    kVertex,
    kIndex,
    kConstant,
    kShaderRead,
    kShaderWrite,
    kTransfer,
    kReadback,
    kAccelerationStructure
};

struct RHIBufferDesc
{
    uint64 Size;
    uint64 Stride = 0;
    RHIBufferUsage Usage;
};

class IRHIBuffer
{
public:
    ~IRHIBuffer() = default;

    virtual void SetName(const StringView& name) = 0;

    RHIBufferDesc GetDesc() const { return mDesc; }

protected:
    RHIBufferDesc mDesc;
};
