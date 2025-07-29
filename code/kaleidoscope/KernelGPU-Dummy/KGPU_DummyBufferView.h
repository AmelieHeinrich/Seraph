//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:52:17
//

#pragma once

#include <KernelGPU/KGPU_BufferView.h>

namespace KGPU
{
    class DummyDevice;

    class DummyBufferView : public IBufferView
    {
    public:
        DummyBufferView(DummyDevice* device, BufferViewDesc desc);
        ~DummyBufferView();

    private:
        DummyDevice* mParentDevice;
    };
}
