//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:11:15
//

#pragma once

#include <RHI/BufferView.h>

#include <Agility/d3d12.h>

#include "D3D12BindlessManager.h"

class D3D12Device;

class D3D12BufferView : public IRHIBufferView
{
public:
    D3D12BufferView(D3D12Device* device, RHIBufferViewDesc desc);
    ~D3D12BufferView();

private:
    D3D12Device* mParentDevice;

    D3D12BindlessAlloc mAlloc;
};
