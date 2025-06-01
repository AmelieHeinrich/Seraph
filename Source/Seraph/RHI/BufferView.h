//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 11:54:27
//

#pragma once

#include <Core/Context.h>

#include "Bindless.h"
#include "Buffer.h"

enum class RHIBufferViewType
{
    kConstant,
    kStructured,
    kStorage
};

struct RHIBufferViewDesc
{
    IRHIBuffer* Buffer;
    RHIBufferViewType Type;

    RHIBufferViewDesc() = default;
    RHIBufferViewDesc(IRHIBuffer* b, RHIBufferViewType t)
        : Buffer(b), Type(t) {}
};

class IRHIBufferView
{
public:
    ~IRHIBufferView() = default;

    RHIBufferViewDesc GetDesc() const { return mDesc; }
    BindlessHandle GetBindlessHandle() const { return mBindless; }

protected:
    RHIBufferViewDesc mDesc;

    BindlessHandle mBindless;
};
