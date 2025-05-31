//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 17:51:03
//

#pragma once

#include <RHI/CommandQueue.h>

class D3D12CommandQueue : public IRHICommandQueue
{
public:
    D3D12CommandQueue() = default;

    IRHICommandBuffer* CreateCommandBuffer(bool singleTime) override { return nullptr; }

    void SubmitAndFlushCommandBuffer(IRHICommandBuffer* cmdBuffer) override {}
};
