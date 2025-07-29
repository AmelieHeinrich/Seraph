//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:16:05
//

#pragma once

#include "KGPU_ShaderModule.h"

namespace KGPU
{
    struct ComputePipelineDesc
    {
        uint PushConstantSize = 0;
        ShaderModule ComputeBytecode;
    
        ComputePipelineDesc() = default;
        ComputePipelineDesc(uint pushConstantSize, ShaderModule bytecode)
            : PushConstantSize(pushConstantSize), ComputeBytecode(bytecode) {}
    };
    
    class IComputePipeline
    {
    public:
        virtual ~IComputePipeline() = default;
    
        ComputePipelineDesc GetDesc() const { return mDesc; }
    protected:
        ComputePipelineDesc mDesc;
    };
}
