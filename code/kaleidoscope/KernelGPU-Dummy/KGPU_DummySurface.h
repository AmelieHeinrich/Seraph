//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:19:38
//

#pragma once

#include <KernelGPU/KGPU_Surface.h>

#include <KernelOS/KOS_Window.h>

namespace KGPU
{  
    class DummyDevice;
    class DummyCommandQueue;

    class DummySurface : public ISurface
    {
    public:
        DummySurface(DummyDevice* device, KOS::IWindow* window, DummyCommandQueue* commandQueue);
        ~DummySurface();

    private:
        DummyDevice* mParentDevice;
    };
}
