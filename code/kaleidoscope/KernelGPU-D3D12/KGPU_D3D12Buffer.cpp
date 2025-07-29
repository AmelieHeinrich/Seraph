//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:31:59
//

#include "KGPU_D3D12Buffer.h"
#include "KGPU_D3D12Device.h"

namespace KGPU
{
    D3D12Buffer::D3D12Buffer(D3D12Device* device, BufferDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

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
        if (Any(desc.Usage & BufferUsage::kShaderWrite)) {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
        if (Any(desc.Usage & BufferUsage::kAccelerationStructure) && mParentDevice->SupportsRaytracing()) {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        D3D12_HEAP_PROPERTIES allocationDesc = {};
        allocationDesc.Type = D3D12_HEAP_TYPE_DEFAULT;
        if (Any(desc.Usage & BufferUsage::kReadback)) allocationDesc.Type = D3D12_HEAP_TYPE_READBACK;
        if (Any(desc.Usage & BufferUsage::kStaging)) allocationDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
        if (Any(desc.Usage & BufferUsage::kConstant)) allocationDesc.Type = D3D12_HEAP_TYPE_UPLOAD;

        HRESULT hr = device->GetDevice()->CreateCommittedResource(&allocationDesc, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mResource));
        KD_ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 texture!");

        KD_WHATEVER("Created D3D12 buffer");
    }

    D3D12Buffer::~D3D12Buffer()
    {
        if (mResource) mResource->Release();
    }

    void D3D12Buffer::SetName(const KC::String& name)
    {
        mResource->SetName(MULTIBYTE_TO_UNICODE(name.c_str()));
    }

    void* D3D12Buffer::Map()
    {
        D3D12_RANGE range;
        range.Begin = 0;
        range.End = mDesc.Size;

        void* ptr;
        mResource->Map(0, &range, &ptr);
        return ptr;
    }

    void D3D12Buffer::Unmap(uint start, uint end)
    {
        D3D12_RANGE range;
        if (start == 0 && end == 0) {
            range.Begin = 0;
            range.End = mDesc.Size;
        } else {
            range.Begin = start;
            range.End = end;
        }

        mResource->Unmap(0, &range);
    }

    uint64 D3D12Buffer::GetAddress()
    {
        return mResource->GetGPUVirtualAddress();
    }
}
