//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 19:12:53
//

#pragma once

#include "ShaderCompiler.h"

struct RHIComputePipelineDesc
{
    uint PushConstantSize = 0;
    ShaderModule ComputeBytecode;

    RHIComputePipelineDesc() = default;
    RHIComputePipelineDesc(uint pushConstantSize, ShaderModule bytecode)
        : PushConstantSize(pushConstantSize), ComputeBytecode(bytecode) {}
};

class IRHIComputePipeline
{
public:
    ~IRHIComputePipeline() = default;

    RHIComputePipelineDesc GetDesc() const { return mDesc; }
protected:
    RHIComputePipelineDesc mDesc;
};
