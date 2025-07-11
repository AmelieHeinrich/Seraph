//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:03:15
//

#include "DummySampler.h"
#include "DummyDevice.h"

DummySampler::DummySampler(DummyDevice* device, RHISamplerDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Dummy sampler!");
}

DummySampler::~DummySampler()
{

}
