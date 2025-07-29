//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:02:21
//

#pragma once

#include <KernelCore/KC_Context.h>

#include "KGPU_Bindless.h"
#include "KGPU_Buffer.h"

namespace KGPU
{
    enum class BufferViewType
    {
        kConstant,
        kStructured,
        kStorage
    };

    struct BufferViewDesc
    {
        IBuffer* Buffer;
        BufferViewType Type;

        BufferViewDesc() = default;
        BufferViewDesc(IBuffer* b, BufferViewType t)
            : Buffer(b), Type(t) {}
    };

    inline bool operator==(const BufferViewDesc& a, const BufferViewDesc& b) {
        return a.Buffer == b.Buffer &&
               a.Type == b.Type;
    }

    class IBufferView
    {
    public:
        virtual ~IBufferView() = default;

        BufferViewDesc GetDesc() const { return mDesc; }
        BindlessHandle GetBindlessHandle() const { return mBindless; }

    protected:
        BufferViewDesc mDesc;

        BindlessHandle mBindless;
    };
}
