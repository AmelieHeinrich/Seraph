//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 19:41:05
//

#pragma once

#include "GraphicsPipeline.h"

using RHIMeshPipelineDesc = RHIGraphicsPipelineDesc;

class IRHIMeshPipeline
{
public:
    ~IRHIMeshPipeline() = default;

    RHIMeshPipelineDesc GetDesc() const { return mDesc; }
protected:
    RHIMeshPipelineDesc mDesc;
};
