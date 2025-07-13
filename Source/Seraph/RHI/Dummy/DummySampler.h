//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:02:44
//

#pragma once

#include <RHI/Sampler.h>

class DummyDevice;

class DummySampler : public IRHISampler
{
public:
    DummySampler(DummyDevice* device, RHISamplerDesc desc);
    ~DummySampler() override;

private:
    DummyDevice* mParentDevice;
};
