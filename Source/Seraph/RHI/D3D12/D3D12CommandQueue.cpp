//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:13:24
//

#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "D3D12Device.h"

D3D12CommandQueue::D3D12CommandQueue(D3D12Device* device, RHICommandQueueType type)
    : mParentDevice(device)
{

}

D3D12CommandQueue::~D3D12CommandQueue()
{

}

void D3D12CommandQueue::SubmitAndFlushCommandBuffer(IRHICommandBuffer* cmdBuffer)
{

}

IRHICommandBuffer* D3D12CommandQueue::CreateCommandBuffer(bool singleTime)
{
    return (new D3D12CommandBuffer(mParentDevice, this, singleTime));
}
