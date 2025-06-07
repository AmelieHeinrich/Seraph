//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:51:03
//

#pragma once

#include <RHI/CommandQueue.h>

#include <Agility/d3d12.h>

class D3D12Device;

class D3D12CommandQueue : public IRHICommandQueue
{
public:
    D3D12CommandQueue(D3D12Device* device, RHICommandQueueType type);
    ~D3D12CommandQueue();

    IRHICommandList* CreateCommandBuffer(bool singleTime) override;

    void SubmitAndFlushCommandBuffer(IRHICommandList* cmdBuffer) override;

public:
    static D3D12_COMMAND_LIST_TYPE TranslateToD3D12List(RHICommandQueueType type);

    ID3D12CommandQueue* GetQueue() { return mQueue; }

private:
    D3D12Device* mParentDevice;
    ID3D12CommandQueue* mQueue = nullptr;
};
