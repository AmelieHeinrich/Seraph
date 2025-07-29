//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:55:44
//

#pragma once

#include <KernelGPU/KGPU_Texture.h>

#include <Agility/d3d12.h>

namespace KGPU
{
    class D3D12Device;

    class D3D12Texture : public ITexture
    {
    public:
        D3D12Texture(TextureDesc desc);
        D3D12Texture(D3D12Device* device, TextureDesc desc);
        ~D3D12Texture();

        void SetName(const KC::String& name) override;

        ID3D12Resource* GetResource() { return mResource; }
    public:
        static DXGI_FORMAT TranslateToDXGIFormat(TextureFormat format);

    private:
        friend class D3D12Surface;

        ID3D12Resource* mResource;
    };
}
