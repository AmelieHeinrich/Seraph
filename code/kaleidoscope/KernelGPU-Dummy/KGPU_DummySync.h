//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:26:51
//

#pragma once

#include <KernelGPU/KGPU_Sync.h>

#include "KGPU_DummySurface.h"
#include "KGPU_DummyCommandQueue.h"

namespace KGPU
{
    class DummySync : public ISync
    {
    public:
        DummySync(DummyDevice* device, DummySurface* surface, DummyCommandQueue* queue);
        ~DummySync();

        uint BeginSynchronize() override;
        void EndSynchronize(ICommandList* submitBuffer) override;
        void PresentSurface() override;
    };
}
