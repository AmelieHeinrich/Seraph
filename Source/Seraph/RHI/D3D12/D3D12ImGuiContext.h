//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 23:00:33
//

#pragma once

#include <RHI/ImGuiContext.h>
#include <Core/Window.h>

#include <Agility/d3d12.h>

#include "D3D12BindlessManager.h"

class D3D12Device;
class D3D12CommandQueue;

class D3D12ImGuiContext : public IRHIImGuiContext
{
public:
    D3D12ImGuiContext(D3D12Device* device, D3D12CommandQueue* queue, Window* window);
    ~D3D12ImGuiContext();

private:
    D3D12Device* mParentDevice;

    D3D12BindlessAlloc mFontAlloc;
};
