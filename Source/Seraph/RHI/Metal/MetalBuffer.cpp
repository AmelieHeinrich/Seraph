//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:00:39
//

#include "MetalBuffer.h"
#include "MetalDevice.h"

#include <Core/String.h>

MetalBuffer::MetalBuffer(MetalDevice* device, RHIBufferDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    mMappedMemory = malloc(desc.Size);

    SERAPH_WHATEVER("Created Metal buffer");
}

MetalBuffer::~MetalBuffer()
{
    free(mMappedMemory);
}

void MetalBuffer::SetName(const std::string& name)
{
    
}

void* MetalBuffer::Map()
{
    return mMappedMemory;
}

void MetalBuffer::Unmap()
{
    
}

uint64 MetalBuffer::GetAddress()
{
    return 0;
}
