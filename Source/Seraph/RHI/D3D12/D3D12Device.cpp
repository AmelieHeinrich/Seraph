//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:54
//

#include "D3D12Device.h"
#include "D3D12CommandQueue.h"

D3D12Device::D3D12Device(bool validationLayers)
{
    SERAPH_INFO("Created D3D12 device!");
}

D3D12Device::~D3D12Device()
{
    
}

IRHICommandQueue* D3D12Device::CreateCommandQueue(RHICommandQueueType type)
{
    return (new D3D12CommandQueue());
}
