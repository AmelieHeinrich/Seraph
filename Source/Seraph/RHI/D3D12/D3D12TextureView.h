//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:52:34
//

#pragma once

#include <RHI/TextureView.h>

#include <Volk/volk.h>

class D3D12Device;

class D3D12TextureView : public IRHITextureView
{
public:
    D3D12TextureView(D3D12Device* device, RHITextureViewDesc viewDesc);
    ~D3D12TextureView();
};
