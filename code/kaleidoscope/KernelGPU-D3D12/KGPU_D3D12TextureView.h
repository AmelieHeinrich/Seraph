//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:01:26
//

#pragma once

#include <KernelGPU/KGPU_TextureView.h>

#include "KGPU_D3D12BindlessManager.h"

namespace KGPU
{
    class D3D12Device;

    class D3D12TextureView : public ITextureView
    {
    public:
        D3D12TextureView(D3D12Device* device, TextureViewDesc viewDesc);
        ~D3D12TextureView();
    
        D3D12BindlessAlloc GetAlloc() { return mAlloc; }
    private:
        D3D12Device* mParentDevice;
    
        D3D12BindlessAlloc mAlloc;
    };
}
