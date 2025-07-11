//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:00:39
//

#include "DummyBuffer.h"
#include "DummyDevice.h"

#include <Core/String.h>

DummyBuffer::DummyBuffer(DummyDevice* device, RHIBufferDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    mMappedMemory = malloc(desc.Size);

    SERAPH_WHATEVER("Created Dummy buffer");
}

DummyBuffer::~DummyBuffer()
{
    free(mMappedMemory);
}

void DummyBuffer::SetName(const std::string& name)
{
    
}

void* DummyBuffer::Map()
{
    return mMappedMemory;
}

void DummyBuffer::Unmap()
{
    
}

uint64 DummyBuffer::GetAddress()
{
    return 0;
}
