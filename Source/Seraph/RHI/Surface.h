//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 11:21:15
//

#pragma once

#include <Core/Window.h>

#include "Texture.h"
#include "TextureView.h"

constexpr uint FRAMES_IN_FLIGHT = 3;

class IRHIDevice;

class IRHISurface
{
public:
    // Constructor takes: Window*
    ~IRHISurface() = default;

    IRHITexture* GetTexture(uint frameIndex) { return mTextures[frameIndex]; }
    IRHITextureView* GetTextureView(uint frameIndex) { return mTextureViews[frameIndex]; }
protected:
    StaticArray<IRHITexture*, FRAMES_IN_FLIGHT> mTextures;
    StaticArray<IRHITextureView*, FRAMES_IN_FLIGHT> mTextureViews;
};
