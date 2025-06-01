//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:00:39
//

#include "D3D12Buffer.h"
#include "D3D12Device.h"

#include <Core/String.h>

D3D12Buffer::D3D12Buffer(D3D12Device* device, RHIBufferDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    if (Any(desc.Usage & RHIBufferUsage::kReadback)) allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
    if (Any(desc.Usage & RHIBufferUsage::kStaging)) allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
    if (Any(desc.Usage & RHIBufferUsage::kConstant)) allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resourceDesc.Width = desc.Size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    if (Any(desc.Usage & RHIBufferUsage::kShaderWrite)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (Any(desc.Usage & RHIBufferUsage::kAccelerationStructure)) resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;

    HRESULT hr = device->GetAllocator()->CreateResource(&allocationDesc, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, &mAllocation, IID_PPV_ARGS(&mResource));
    ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 texture!");

    SERAPH_WHATEVER("Created D3D12 buffer");
}

D3D12Buffer::~D3D12Buffer()
{
    if (mAllocation) mAllocation->Release();
}

void D3D12Buffer::SetName(const StringView& name)
{
    mAllocation->SetName(MULTIBYTE_TO_UNICODE(name.data()));
    mResource->SetName(MULTIBYTE_TO_UNICODE(name.data()));
}

void* D3D12Buffer::Map()
{
    D3D12_RANGE range;
    range.Begin = 0;
    range.End = 0;

    void* ptr;
    mResource->Map(0, &range, &ptr);
    return ptr;
}

void D3D12Buffer::Unmap()
{
    D3D12_RANGE range;
    range.Begin = 0;
    range.End = 0;

    mResource->Unmap(0, &range);
}

uint64 D3D12Buffer::GetAddress()
{
    return mResource->GetGPUVirtualAddress();
}
