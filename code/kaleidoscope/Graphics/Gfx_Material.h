//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 12:40:25
//

#pragma once

#include <KDAsset/KDA_MeshLoader.h>
#include <KDAsset/KDA_TextureLoader.h>
#include <KernelGPU/KGPU_Device.h>

namespace Gfx
{
    class Material
    {
    public:
        Material() = default;
        Material(const KDA::MeshMaterial& material);
        ~Material();

        void FromMaterial(const KDA::MeshMaterial& material);

        KGPU::ITextureView* GetAlbedoView() { return mAlbedo.View; }
        KGPU::ITextureView* GetNormalView() { return mNormal.View; }
        KGPU::ITextureView* GetMRView() { return mMetallicRoughness.View; }
        KGPU::ITextureView* GetEmissiveView() { return mEmissive.View; }

        bool IsOpaque() const { return mIsOpaque; }
    private:
        struct TexturePair {
            KGPU::ITexture* Texture = nullptr;
            KGPU::ITextureView* View = nullptr;
        };

        TexturePair mAlbedo;
        TexturePair mNormal;
        TexturePair mMetallicRoughness;
        TexturePair mEmissive;
        bool mIsOpaque;
    };
}
