//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:52:34
//

#pragma once

#include <RHI/TextureView.h>

class DummyDevice;

class DummyTextureView : public IRHITextureView
{
public:
    DummyTextureView(DummyDevice* device, RHITextureViewDesc viewDesc);
    ~DummyTextureView() override;

    uint64 GetTextureID() override { return 0; }
private:
    DummyDevice* mParentDevice;
};
