//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:01:37
//

#pragma once

#include <KernelCore/KC_Context.h>

namespace KGPU
{
    enum class BufferUsage
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
    ENUM_CLASS_FLAGS(BufferUsage);

    struct BufferDesc
    {
        uint64 Size;
        uint64 Stride = 0;
        BufferUsage Usage;

        BufferDesc() = default;
        BufferDesc(uint64 size, uint64 stride, BufferUsage usage)
            : Size(size), Stride(stride), Usage(usage) {}
    };

    class IBuffer
    {
    public:
        virtual ~IBuffer() = default;

        virtual void SetName(const KC::String& name) = 0;

        BufferDesc GetDesc() const { return mDesc; }

        virtual void* Map() = 0;
        virtual void Unmap(uint start = 0, uint end = 0) = 0;
        virtual uint64 GetAddress() = 0;
    protected:
        BufferDesc mDesc;
    };
}
