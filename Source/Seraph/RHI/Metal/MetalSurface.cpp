//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:47:27
//

#include "MetalSurface.h"
#include "MetalDevice.h"
#include "MetalCommandQueue.h"
#include "MetalTexture.h"
#include "MetalTextureView.h"

MetalSurface::MetalSurface(MetalDevice* device, Window* window, MetalCommandQueue* commandQueue)
    : mParentDevice(device)
{
    SDL_Window* sdlWindow = window->GetWindow();
    int width, height;
    SDL_GetWindowSize(sdlWindow, &width, &height);
    SDL_SetWindowTitle(sdlWindow, "Seraph | Metal");

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        RHITextureDesc desc = {};
        desc.Reserved = true;
        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.Usage = RHITextureUsage::kRenderTarget;
        desc.Format = RHITextureFormat::kR8G8B8A8_UNORM;

        MetalTexture* texture = new MetalTexture(desc);
    
        mTextures[i] = texture;
        mTextureViews[i] = new MetalTextureView(device, RHITextureViewDesc(texture, RHITextureViewType::kRenderTarget));
    }

    SERAPH_WHATEVER("Created Metal surface");
}

MetalSurface::~MetalSurface()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mTextureViews[i];
        delete mTextures[i];
    }
}
