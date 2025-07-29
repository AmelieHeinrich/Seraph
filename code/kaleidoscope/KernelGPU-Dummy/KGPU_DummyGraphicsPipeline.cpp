//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:12:13
//

#include "KGPU_DummyGraphicsPipeline.h"
#include "KGPU_DummyDevice.h"
#include "KGPU_DummyTexture.h"

namespace KGPU
{
    DummyGraphicsPipeline::DummyGraphicsPipeline(DummyDevice* device, GraphicsPipelineDesc desc)
    {
        mDesc = desc;

        KD_WHATEVER("Created Dummy graphics pipeline");
    }
    
    DummyGraphicsPipeline::~DummyGraphicsPipeline()
    {

    }
}
