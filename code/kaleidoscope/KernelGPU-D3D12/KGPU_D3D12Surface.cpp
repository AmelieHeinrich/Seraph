//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:23:43
//

#include "KGPU_D3D12Surface.h"
#include "KGPU_D3D12Device.h"
#include "KGPU_D3D12CommandQueue.h"
#include "KGPU_D3D12Texture.h"
#include "KGPU_D3D12TextureView.h"

namespace KGPU
{
    D3D12Surface::D3D12Surface(D3D12Device* device, KOS::IWindow* window, D3D12CommandQueue* commandQueue)
        : mParentDevice(device)
    {
        HWND hwnd = (HWND)window->GetGPUSurface();
    
        int width, height;
        window->GetSize(width, height);
        
        KC::String title = window->GetTitle();
        title += " | D3D12";
        window->SetTitle(title.c_str());
    
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
        
            TextureDesc desc = {};
            desc.Reserved = true;
            desc.Width = width;
            desc.Height = height;
            desc.Depth = 1;
            desc.MipLevels = 1;
            desc.Usage = TextureUsage::kRenderTarget;
            desc.Format = TextureFormat::kR8G8B8A8_UNORM;
        
            D3D12Texture* texture = KC_NEW(D3D12Texture, desc);
            texture->mResource = backbuffer;
        
            mTextures[i] = texture;
            mTextureViews[i] = KC_NEW(D3D12TextureView, device, TextureViewDesc(texture, TextureViewType::kRenderTarget));
        }
    
        KD_WHATEVER("Created D3D12 surface");
    }
    
    D3D12Surface::~D3D12Surface()
    {
        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            KC_DELETE(mTextureViews[i]);
            KC_DELETE(mTextures[i]);
        }
    }
}
