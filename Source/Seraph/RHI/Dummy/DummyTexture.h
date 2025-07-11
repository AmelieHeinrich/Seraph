//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:50:04
//

#pragma once

#include <RHI/Texture.h>

class DummyDevice;

class DummyTexture : public IRHITexture
{
public:
    DummyTexture(RHITextureDesc desc);
    DummyTexture(DummyDevice* device, RHITextureDesc desc);
    ~DummyTexture();

    void SetName(const String& name) override;

private:
    friend class DummySurface;
};
