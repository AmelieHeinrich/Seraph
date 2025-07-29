//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:01:26
//

#pragma once

#include <KernelGPU/KGPU_TextureView.h>

namespace KGPU
{
    class DummyDevice;

    class DummyTextureView : public ITextureView
    {
    public:
        DummyTextureView(DummyDevice* device, TextureViewDesc viewDesc);
        ~DummyTextureView();
    };
}
