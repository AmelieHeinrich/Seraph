//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:23:43
//

#include "KGPU_DummySurface.h"
#include "KGPU_DummyDevice.h"
#include "KGPU_DummyCommandQueue.h"
#include "KGPU_DummyTexture.h"
#include "KGPU_DummyTextureView.h"

namespace KGPU
{
    DummySurface::DummySurface(DummyDevice* device, KOS::IWindow* window, DummyCommandQueue* commandQueue)
        : mParentDevice(device)
    {
        int width, height;
        window->GetSize(width, height);
        
        KC::String title = window->GetTitle();
        title += " | Dummy";
        window->SetTitle(title.c_str());

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            TextureDesc desc = {};
            desc.Reserved = true;
            desc.Width = width;
            desc.Height = height;
            desc.Depth = 1;
            desc.MipLevels = 1;
            desc.Usage = TextureUsage::kRenderTarget;
            desc.Format = TextureFormat::kR8G8B8A8_UNORM;
        
            DummyTexture* texture = KC_NEW(DummyTexture, desc);

            mTextures[i] = texture;
            mTextureViews[i] = KC_NEW(DummyTextureView, device, TextureViewDesc(texture, TextureViewType::kRenderTarget));
        }
    
        KD_WHATEVER("Created Dummy surface");
    }
    
    DummySurface::~DummySurface()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            KC_DELETE(mTextureViews[i]);
            KC_DELETE(mTextures[i]);
        }
    }
}
