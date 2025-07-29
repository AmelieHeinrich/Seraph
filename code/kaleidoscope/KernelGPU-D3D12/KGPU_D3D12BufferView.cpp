//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:53:20
//

#include "KGPU_D3D12BufferView.h"
#include "KGPU_D3D12Device.h"

namespace KGPU
{
    D3D12BufferView::D3D12BufferView(D3D12Device* device, BufferViewDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        switch (desc.Type) {
            case BufferViewType::kConstant: {
                mAlloc = device->GetBindlessManager()->WriteBufferCBV(this);
                break;
            }
            case BufferViewType::kStorage: {
                mAlloc = device->GetBindlessManager()->WriteBufferUAV(this);
                break;
            }
            case BufferViewType::kStructured: {
                mAlloc = device->GetBindlessManager()->WriteBufferSRV(this);
                break;
            }
        }

        mBindless.Index = mAlloc.Index;

        KD_WHATEVER("Created D3D12 buffer view");
    }

    D3D12BufferView::~D3D12BufferView()
    {
        mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mAlloc);
    }
}
