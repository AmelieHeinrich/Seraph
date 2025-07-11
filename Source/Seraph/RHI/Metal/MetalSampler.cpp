//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:03:15
//

#include "MetalSampler.h"
#include "MetalDevice.h"

MetalSampler::MetalSampler(MetalDevice* device, RHISamplerDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Metal sampler!");
}

MetalSampler::~MetalSampler()
{

}
