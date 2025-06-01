//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:46:25
//

#pragma once

#include <RHI/Surface.h>

#include <Agility/d3d12.h>
#include <dxgi1_6.h>

class D3D12Device;
class D3D12CommandQueue;

class D3D12Surface : public IRHISurface
{
public:
    D3D12Surface(D3D12Device* device, Window* window, D3D12CommandQueue* commandQueue);
    ~D3D12Surface();

public:
    IDXGISwapChain4* GetSwapchain() { return mSwapchain; }

private:
    D3D12Device* mParentDevice;

    IDXGISwapChain4* mSwapchain;
};
