//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:47:27
//

#include "D3D12Surface.h"
#include "D3D12Texture.h"
#include "D3D12TextureView.h"

D3D12Surface::D3D12Surface(D3D12Device* device, Window* window)
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mTextures[i] = new D3D12Texture(device, {});
        mTextureViews[i] = new D3D12TextureView(device, {});
    }
}

D3D12Surface::~D3D12Surface()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mTextureViews[i];
        delete mTextures[i];
    }
}
