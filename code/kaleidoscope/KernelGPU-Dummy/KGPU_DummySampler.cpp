//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:09:19
//

#include "KGPU_DummySampler.h"
#include "KGPU_DummyDevice.h"

namespace KGPU
{
    DummySampler::DummySampler(DummyDevice* device, SamplerDesc desc)
    {
        mDesc = desc;

        KD_WHATEVER("Created Dummy sampler!");
    }

    DummySampler::~DummySampler()
    {
    }
}
