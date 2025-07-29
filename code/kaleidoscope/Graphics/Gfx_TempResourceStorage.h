//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:50:11
//

#pragma once

#include "Gfx_Manager.h"

#include <KernelCore/KC_Context.h>

namespace Gfx
{
    class TempResourceStorage
    {
    public:
        static void Clean();

        static KGPU::IBuffer* CreateBuffer(KGPU::BufferDesc desc);
        static KGPU::ITexture* CreateTexture(KGPU::TextureDesc desc);
        static KGPU::ITextureView* CreateTextureView(KGPU::TextureViewDesc desc);
        static KGPU::IBufferView* CreateBufferView(KGPU::BufferViewDesc desc);
        static KGPU::ISampler* CreateSampler(KGPU::SamplerDesc desc);
    private:
        static struct Data {
            KC::Array<KGPU::IBuffer*> mBuffers;
            KC::Array<KGPU::ITexture*> mTextures;
            KC::Array<KGPU::ITextureView*> mTextureViews;
            KC::Array<KGPU::IBufferView*> mBufferViews;
            KC::Array<KGPU::ISampler*> mSamplers;
        } sData;
    };
}
