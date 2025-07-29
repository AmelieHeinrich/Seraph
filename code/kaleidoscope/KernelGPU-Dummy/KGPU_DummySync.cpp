//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:27:36
//

#include "KGPU_DummySync.h"
#include "KGPU_DummyDevice.h"
#include "KGPU_DummyCommandQueue.h"
#include "KGPU_DummyCommandList.h"

namespace KGPU
{
    DummySync::DummySync(DummyDevice* device, DummySurface* surface, DummyCommandQueue* queue)
    {
        KD_WHATEVER("Created Dummy F2F sync");
    }

    DummySync::~DummySync()
    {
    }

    uint DummySync::BeginSynchronize()
    {
        return 0;
    }

    void DummySync::EndSynchronize(ICommandList* submitBuffer)
    {
    }

    void DummySync::PresentSurface()
    {
    }
}
