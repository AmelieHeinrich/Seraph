//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:02:44
//

#include "KGPU_D3D12TextureView.h"
#include "KGPU_D3D12Device.h"

namespace KGPU
{
    D3D12TextureView::D3D12TextureView(D3D12Device* device, TextureViewDesc viewDesc)
        : mParentDevice(device), mAlloc({})
    {
        mDesc = viewDesc;

        switch (mDesc.Type) {
            case TextureViewType::kRenderTarget: {
                mAlloc = mParentDevice->GetBindlessManager()->WriteRTV(this);
                break;
            }
            case TextureViewType::kDepthTarget: {
                mAlloc = mParentDevice->GetBindlessManager()->WriteDSV(this);
                break;
            }
            case TextureViewType::kShaderRead: {
                mAlloc = mParentDevice->GetBindlessManager()->WriteTextureSRV(this);
                break;
            }
            case TextureViewType::kShaderWrite: {
                mAlloc = mParentDevice->GetBindlessManager()->WriteTextureUAV(this);
                break;
            }
        }

        mBindless.Index = mAlloc.Index;

        KD_WHATEVER("Created D3D12 texture view");
    }

    D3D12TextureView::~D3D12TextureView()
    {
        switch (mDesc.Type) {
            case TextureViewType::kRenderTarget: {
                mParentDevice->GetBindlessManager()->FreeRTV(mAlloc);
                break;
            }
            case TextureViewType::kDepthTarget: {
                mParentDevice->GetBindlessManager()->FreeDSV(mAlloc);
                break;
            }
            case TextureViewType::kShaderRead:
            case TextureViewType::kShaderWrite: {
                mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mAlloc);
                break;
            }
        }
    }
}
