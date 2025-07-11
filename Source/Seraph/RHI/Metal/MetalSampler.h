//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:02:44
//

#pragma once

#include <RHI/Sampler.h>
#include <MetalCPP/Metal/Metal.hpp>

class MetalDevice;

class MetalSampler : public IRHISampler
{
public:
    MetalSampler(MetalDevice* device, RHISamplerDesc desc);
    ~MetalSampler();

private:
    MetalDevice* mParentDevice;
};
