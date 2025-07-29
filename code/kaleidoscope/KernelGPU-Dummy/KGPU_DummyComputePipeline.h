//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:43:49
//

#pragma once

#include <KernelGPU/KGPU_ComputePipeline.h>

namespace KGPU
{
    class DummyDevice;

    class DummyComputePipeline : public IComputePipeline
    {
    public:
        DummyComputePipeline(DummyDevice* device, ComputePipelineDesc desc);
        ~DummyComputePipeline();
    };
}
