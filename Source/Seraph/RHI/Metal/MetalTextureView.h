//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:52:34
//

#pragma once

#include <RHI/TextureView.h>

class MetalDevice;

class MetalTextureView : public IRHITextureView
{
public:
    MetalTextureView(MetalDevice* device, RHITextureViewDesc viewDesc);
    ~MetalTextureView();

    uint64 GetTextureID() override { return 0; }
private:
    MetalDevice* mParentDevice;
};
