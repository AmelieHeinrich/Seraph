//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:37
//

#pragma once

#include <RHI/Device.h>

class D3D12Device : public IRHIDevice
{
public:
    D3D12Device(bool validationLayers);
    ~D3D12Device();

    IRHISurface* CreateSurface(Window* window) override { return nullptr; }
    IRHITexture* CreateTexture(RHITextureDesc desc) override { return nullptr; }
    IRHITextureView* CreateTextureView(RHITextureViewDesc desc) override { return nullptr; }
    IRHICommandQueue* CreateCommandQueue(RHICommandQueueType type) override { return nullptr; }
};
