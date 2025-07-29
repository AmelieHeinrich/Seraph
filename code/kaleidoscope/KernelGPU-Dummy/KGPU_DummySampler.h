//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:08:46
//

#pragma once

#include <KernelGPU/KGPU_Sampler.h>

namespace KGPU
{
    class DummyDevice;

    class DummySampler : public ISampler
    {
    public:
        DummySampler(DummyDevice* device, SamplerDesc desc);
        ~DummySampler();
    };
}
