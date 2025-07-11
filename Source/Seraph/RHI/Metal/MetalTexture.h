//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:50:04
//

#pragma once

#include <RHI/Texture.h>

class MetalDevice;

class MetalTexture : public IRHITexture
{
public:
    MetalTexture(RHITextureDesc desc);
    MetalTexture(MetalDevice* device, RHITextureDesc desc);
    ~MetalTexture();

    void SetName(const std::string& name) override;

private:
    friend class MetalSurface;
};
