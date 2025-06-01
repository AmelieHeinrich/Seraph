//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:47:27
//

#include "D3D12Surface.h"
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12Texture.h"
#include "D3D12TextureView.h"

D3D12Surface::D3D12Surface(D3D12Device* device, Window* window, D3D12CommandQueue* commandQueue)
    : mParentDevice(device)
{
    SDL_Window* sdlWindow = window->GetWindow();
    HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

    int width, height;
    SDL_GetWindowSize(sdlWindow, &width, &height);
    SDL_SetWindowTitle(sdlWindow, "Seraph | D3D12");

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = FRAMES_IN_FLIGHT;
    desc.Scaling = DXGI_SCALING_NONE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Width = width;
    desc.Height = height;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    IDXGISwapChain1* temp;
    HRESULT result = mParentDevice->GetFactory()->CreateSwapChainForHwnd(commandQueue->GetQueue(), hwnd, &desc, nullptr, nullptr, &temp);
    temp->QueryInterface(&mSwapchain);
    temp->Release();

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        ID3D12Resource* backbuffer = nullptr;
        mSwapchain->GetBuffer(i, IID_PPV_ARGS(&backbuffer));

        RHITextureDesc desc = {};
        desc.Reserved = true;
        desc.Width = width;
        desc.Height = height;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.Usage = RHITextureUsage::kRenderTarget;
        desc.Format = RHITextureFormat::kR8G8B8A8_UNORM;

        D3D12Texture* texture = new D3D12Texture(desc);
        texture->mResource = backbuffer;
    
        mTextures[i] = texture;
        mTextureViews[i] = new D3D12TextureView(device, RHITextureViewDesc(texture, RHITextureViewType::kRenderTarget));
    }

    SERAPH_WHATEVER("Created D3D12 surface");
}

D3D12Surface::~D3D12Surface()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mTextureViews[i];
        delete mTextures[i];
    }
}
