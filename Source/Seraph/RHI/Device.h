//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:26:45
//

#pragma once

#include <Core/Context.h>

#include "Surface.h"
#include "Texture.h"
#include "TextureView.h"

enum class RHIBackend
{
    kVulkan,
    kD3D12
};

class IRHIDevice
{
public:
    ~IRHIDevice() = default;

    static IRHIDevice* CreateDevice(RHIBackend backend, bool validationLayers);

    virtual IRHISurface* CreateSurface(Window* window) = 0;
    virtual IRHITexture* CreateTexture(RHITextureDesc desc) = 0;
    virtual IRHITextureView* CreateTextureView(RHITextureViewDesc desc) = 0;
protected:
    IRHIDevice() = default;
};
