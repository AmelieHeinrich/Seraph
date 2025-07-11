//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:04:31
//

#pragma once

#include <RHI/ComputePipeline.h>

class DummyDevice;

class DummyComputePipeline : public IRHIComputePipeline
{
public:
    DummyComputePipeline(DummyDevice* device, RHIComputePipelineDesc desc);
    ~DummyComputePipeline();
};
