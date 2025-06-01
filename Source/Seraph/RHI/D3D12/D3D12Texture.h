//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:50:04
//

#pragma once

#include <RHI/Texture.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12Texture : public IRHITexture
{
public:
    D3D12Texture() = default;
    D3D12Texture(D3D12Device* device, RHITextureDesc desc);
    ~D3D12Texture();

    void SetName(const StringView& name) override;
};
