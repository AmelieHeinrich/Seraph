//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:47:27
//

#include "MetalSurface.h"
#include "MetalDevice.h"
#include "MetalCommandQueue.h"
#include "MetalTexture.h"
#include "MetalTextureView.h"

#include <QuartzCore/QuartzCore.h>

MetalSurface::MetalSurface(MetalDevice* device, Window* window, MetalCommandQueue* commandQueue)
    : mParentDevice(device)
{
    SDL_Window* sdlWindow = window->GetWindow();
    int width, height;
    SDL_GetWindowSize(sdlWindow, &width, &height);
    SDL_SetWindowTitle(sdlWindow, "Seraph | Metal");

    mView = SDL_Metal_CreateView(sdlWindow);
    void* layerPtr = SDL_Metal_GetLayer(mView);
    mLayer = static_cast<CA::MetalLayer*>(layerPtr);

    mLayer->setDevice(device->GetDevice());
    mLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    mLayer->setDrawableSize(CGSizeMake(width, height));
    mLayer->setMaximumDrawableCount(FRAMES_IN_FLIGHT);
    mLayer->setAllowsNextDrawableTimeout(false);
    mLayer->setDisplaySyncEnabled(false);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        RHITextureDesc desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.Usage = RHITextureUsage::kRenderTarget;
        desc.Format = RHITextureFormat::kB8G8R8A8_UNORM;

        mTextures[i] = new MetalTexture(device, desc);
        mTextures[i]->SetName("Frame in flight " + std::to_string(i));
        mTextureViews[i] = new MetalTextureView(device, RHITextureViewDesc(mTextures[i], RHITextureViewType::kRenderTarget));
    }
}

MetalSurface::~MetalSurface()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mTextureViews[i];
        delete mTextures[i];
    }
    SDL_Metal_DestroyView(mView);
}
