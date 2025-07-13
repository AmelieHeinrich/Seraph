//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:00:39
//

#include "MetalBuffer.h"
#include "MetalDevice.h"

#include <Core/String.h>

MTL::ResourceOptions GetMetalResourceOptions(RHIBufferUsage usage)
{
    MTL::ResourceOptions options = MTL::ResourceStorageModePrivate; // default: GPU-only

    if (Any(usage & RHIBufferUsage::kStaging) || Any(usage & RHIBufferUsage::kReadback) || Any(usage & RHIBufferUsage::kConstant)) {
        options = MTL::ResourceStorageModeShared;
    }

    // Optional: CPU cache mode
    if (Any(usage & RHIBufferUsage::kStaging)) {
        options |= MTL::ResourceCPUCacheModeWriteCombined;
    } else {
        options |= MTL::ResourceCPUCacheModeDefaultCache;
    }

    return options;
}

MetalBuffer::MetalBuffer(MetalDevice* device, RHIBufferDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    mBuffer = device->GetDevice()->newBuffer(desc.Size, GetMetalResourceOptions(desc.Usage));
    if (!mBuffer) {
        SERAPH_ERROR("Failed to create buffer!");
    }

    SERAPH_WHATEVER("Created Metal buffer");
}

MetalBuffer::~MetalBuffer()
{
}

void MetalBuffer::SetName(const std::string& name)
{
    mLabel = NS::String::alloc()->init(name.c_str(), NS::StringEncoding::ASCIIStringEncoding);
    mBuffer->setLabel(mLabel);
}

void* MetalBuffer::Map()
{
    if (mBuffer->storageMode() == MTL::StorageModeShared ||
        mBuffer->storageMode() == MTL::StorageModeManaged) {
        return mBuffer->contents();
    } else {
        // Private / memoryless storage can't be mapped
        return nullptr;
    }
}

void MetalBuffer::Unmap()
{
    // TODO: Synchronize resource
}

uint64 MetalBuffer::GetAddress()
{
    return mBuffer->gpuAddress();
}
