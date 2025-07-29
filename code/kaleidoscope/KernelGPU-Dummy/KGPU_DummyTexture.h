//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:55:44
//

#pragma once

#include <KernelGPU/KGPU_Texture.h>

namespace KGPU
{
    class DummyDevice;

    class DummyTexture : public ITexture
    {
    public:
        DummyTexture(TextureDesc desc);
        DummyTexture(DummyDevice* device, TextureDesc desc);
        ~DummyTexture();

        void SetName(const KC::String& name) override;

    private:
        friend class DummySurface;
    };
}
