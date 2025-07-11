//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:58:11
//

#pragma once

#include <RHI/GraphicsPipeline.h>

class DummyDevice;

class DummyGraphicsPipeline : public IRHIGraphicsPipeline
{
public:
    DummyGraphicsPipeline(DummyDevice* device, RHIGraphicsPipelineDesc desc);
    ~DummyGraphicsPipeline();
};
