//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:26:18
//

#pragma once

#include <KernelGPU/KGPU_Buffer.h>

namespace KGPU
{
    class DummyDevice;

    class DummyBuffer : public IBuffer
    {
    public:
        DummyBuffer(DummyDevice* device, BufferDesc desc);
        ~DummyBuffer();

        void SetName(const KC::String& name) override;

        void* Map() override;
        void Unmap(uint start = 0, uint end = 0) override;

        uint64 GetAddress() override;

    private:
        DummyDevice* mParentDevice;
        void* mMemory;
    };
}
