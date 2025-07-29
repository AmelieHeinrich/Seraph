//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:11:40
//

#pragma once

#include <KernelGPU/KGPU_GraphicsPipeline.h>

namespace KGPU
{
    class DummyDevice;

    class DummyGraphicsPipeline : public IGraphicsPipeline
    {
    public:
        DummyGraphicsPipeline(DummyDevice* device, GraphicsPipelineDesc desc);
        ~DummyGraphicsPipeline();
    };
}
