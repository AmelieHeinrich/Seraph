//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:31:59
//

#include "KGPU_DummyBuffer.h"
#include "KGPU_DummyDevice.h"

namespace KGPU
{
    DummyBuffer::DummyBuffer(DummyDevice* device, BufferDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        mMemory = KD_MEMORY_ALLOC(desc.Size);

        KD_WHATEVER("Created Dummy buffer");
    }

    DummyBuffer::~DummyBuffer()
    {
        KD_MEMORY_FREE(mMemory);   
    }

    void DummyBuffer::SetName(const KC::String& name)
    {
    }

    void* DummyBuffer::Map()
    {
        return mMemory;
    }

    void DummyBuffer::Unmap(uint start, uint end)
    {
    }

    uint64 DummyBuffer::GetAddress()
    {
        return 0;
    }
}
