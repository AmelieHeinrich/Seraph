//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:19:38
//

#pragma once

#include <KernelGPU/KGPU_Surface.h>

#include <Agility/d3d12.h>
#include <dxgi1_6.h>
#include <KernelOS/KOS_Window.h>

namespace KGPU
{  
    class D3D12Device;
    class D3D12CommandQueue;

    class D3D12Surface : public ISurface
    {
    public:
        D3D12Surface(D3D12Device* device, KOS::IWindow* window, D3D12CommandQueue* commandQueue);
        ~D3D12Surface();

    public:
        IDXGISwapChain4* GetSwapchain() { return mSwapchain; }

    private:
        D3D12Device* mParentDevice;

        IDXGISwapChain4* mSwapchain;
    };
}
