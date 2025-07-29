//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:53:20
//

#include "KGPU_DummyBufferView.h"
#include "KGPU_DummyDevice.h"

namespace KGPU
{
    DummyBufferView::DummyBufferView(DummyDevice* device, BufferViewDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        KD_WHATEVER("Created Dummy buffer view");
    }

    DummyBufferView::~DummyBufferView()
    {
    }
}
