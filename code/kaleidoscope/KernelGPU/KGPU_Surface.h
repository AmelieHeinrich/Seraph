//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:07:02
//

#pragma once

#include "KGPU_Texture.h"
#include "KGPU_TextureView.h"

namespace KGPU
{
    constexpr uint FRAMES_IN_FLIGHT = 3;

    class ISurface
    {
    public:
        // Constructor takes: Window*
        virtual ~ISurface() = default;
    
        ITexture* GetTexture(uint frameIndex) { return mTextures[frameIndex]; }
        ITextureView* GetTextureView(uint frameIndex) { return mTextureViews[frameIndex]; }
    protected:
        ITexture* mTextures[FRAMES_IN_FLIGHT];
        ITextureView* mTextureViews[FRAMES_IN_FLIGHT];
    };
}
