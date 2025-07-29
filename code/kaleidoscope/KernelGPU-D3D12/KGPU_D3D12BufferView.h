//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:52:17
//

#pragma once

#include <KernelGPU/KGPU_BufferView.h>

#include <Agility/d3d12.h>

#include "KGPU_D3D12BindlessManager.h"

namespace KGPU
{
    class D3D12Device;

    class D3D12BufferView : public IBufferView
    {
    public:
        D3D12BufferView(D3D12Device* device, BufferViewDesc desc);
        ~D3D12BufferView();

    private:
        D3D12Device* mParentDevice;

        D3D12BindlessAlloc mAlloc;
    };
}
