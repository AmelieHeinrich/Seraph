//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:51:03
//

#pragma once

#include <RHI/CommandQueue.h>

class D3D12Device;

class D3D12CommandQueue : public IRHICommandQueue
{
public:
    D3D12CommandQueue(D3D12Device* device, RHICommandQueueType type);
    ~D3D12CommandQueue();

    IRHICommandBuffer* CreateCommandBuffer(bool singleTime) override;

    void SubmitAndFlushCommandBuffer(IRHICommandBuffer* cmdBuffer) override;
private:
    D3D12Device* mParentDevice;
};
