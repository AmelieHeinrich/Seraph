//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:11:33
//

#include "D3D12BufferView.h"
#include "D3D12Device.h"

D3D12BufferView::D3D12BufferView(D3D12Device* device, RHIBufferViewDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    switch (desc.Type) {
        case RHIBufferViewType::kConstant: {
            mAlloc = device->GetBindlessManager()->WriteBufferCBV(this);
            break;
        }
        case RHIBufferViewType::kStorage: {
            mAlloc = device->GetBindlessManager()->WriteBufferUAV(this);
            break;
        }
        case RHIBufferViewType::kStructured: {
            mAlloc = device->GetBindlessManager()->WriteBufferSRV(this);
            break;
        }
    }

    mBindless.Index = mAlloc.Index;

    SERAPH_WHATEVER("Created D3D12 buffer view");
}

D3D12BufferView::~D3D12BufferView()
{
    mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mAlloc);
}
