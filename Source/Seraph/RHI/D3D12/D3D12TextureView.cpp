//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:53:40
//

#include "D3D12TextureView.h"
#include "D3D12Device.h"

D3D12TextureView::D3D12TextureView(D3D12Device* device, RHITextureViewDesc viewDesc)
{
    switch (mDesc.Type) {
        case RHITextureViewType::kRenderTarget: {
            mAlloc = mParentDevice->GetBindlessManager()->WriteRTV(this);
            break;
        }
        case RHITextureViewType::kDepthTarget: {
            mAlloc = mParentDevice->GetBindlessManager()->WriteDSV(this);
            break;
        }
        case RHITextureViewType::kShaderRead: {
            mAlloc = mParentDevice->GetBindlessManager()->WriteTextureSRV(this);
            break;
        }
        case RHITextureViewType::kShaderWrite: {
            mAlloc = mParentDevice->GetBindlessManager()->WriteTextureUAV(this);
            break;
        }
    }
    
    mBindless.Index = mAlloc.Index;
}

D3D12TextureView::~D3D12TextureView()
{
    switch (mDesc.Type) {
        case RHITextureViewType::kRenderTarget: {
            mParentDevice->GetBindlessManager()->FreeRTV(mAlloc);
            break;
        }
        case RHITextureViewType::kDepthTarget: {
            mParentDevice->GetBindlessManager()->FreeDSV(mAlloc);
            break;
        }
        case RHITextureViewType::kShaderRead:
        case RHITextureViewType::kShaderWrite: {
            mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mAlloc);
            break;
        }
    }
}
