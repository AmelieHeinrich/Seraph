//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:02:44
//

#include "KGPU_DummyTextureView.h"
#include "KGPU_DummyDevice.h"

namespace KGPU
{
    DummyTextureView::DummyTextureView(DummyDevice* device, TextureViewDesc viewDesc)
    {
        mDesc = viewDesc;

        KD_WHATEVER("Created Dummy texture view");
    }

    DummyTextureView::~DummyTextureView()
    {
    }
}
