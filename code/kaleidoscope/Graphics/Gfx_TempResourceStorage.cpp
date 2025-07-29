//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:53:45
//

#include "Gfx_TempResourceStorage.h"

namespace Gfx
{
    TempResourceStorage::Data TempResourceStorage::sData;

    void TempResourceStorage::Clean()
    {
        for (auto& resource :sData.mBuffers) {
            if (resource) KC_DELETE(resource);
        }
        for (auto& resource :sData.mTextures) {
            if (resource) KC_DELETE(resource);
        }
        for (auto& resource :sData.mTextureViews) {
            if (resource) KC_DELETE(resource);
        }
        for (auto& resource :sData.mBufferViews) {
            if (resource) KC_DELETE(resource);
        }
        for (auto& resource :sData.mSamplers) {
            if (resource) KC_DELETE(resource);
        }

        sData.mBuffers.clear();
        sData.mTextures.clear();
        sData.mTextureViews.clear();
        sData.mBufferViews.clear();
        sData.mSamplers.clear();
    }

    KGPU::IBuffer* TempResourceStorage::CreateBuffer(KGPU::BufferDesc desc)
    {
        sData.mBuffers.push_back(Manager::GetDevice()->CreateBuffer(desc));
        return sData.mBuffers.back();
    }

    KGPU::ITexture* TempResourceStorage::CreateTexture(KGPU::TextureDesc desc)
    {
        sData.mTextures.push_back(Manager::GetDevice()->CreateTexture(desc));
        return sData.mTextures.back();
    }

    KGPU::ITextureView* TempResourceStorage::CreateTextureView(KGPU::TextureViewDesc desc)
    {
        sData.mTextureViews.push_back(Manager::GetDevice()->CreateTextureView(desc));
        return sData.mTextureViews.back();
    }

    KGPU::IBufferView* TempResourceStorage::CreateBufferView(KGPU::BufferViewDesc desc)
    {
        sData.mBufferViews.push_back(Manager::GetDevice()->CreateBufferView(desc));
        return sData.mBufferViews.back();
    }

    KGPU::ISampler* TempResourceStorage::CreateSampler(KGPU::SamplerDesc desc)
    {
        sData.mSamplers.push_back(Manager::GetDevice()->CreateSampler(desc));
        return sData.mSamplers.back();
    }
}
