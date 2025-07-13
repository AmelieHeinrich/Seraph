//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:04:31
//

#pragma once

#include <RHI/ComputePipeline.h>

class MetalDevice;

class MetalComputePipeline : public IRHIComputePipeline
{
public:
    MetalComputePipeline(MetalDevice* device, RHIComputePipelineDesc desc);
    ~MetalComputePipeline() override;
};
