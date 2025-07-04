//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:55:41
//

#pragma once

#include <RHI/F2FSync.h>

#include "D3D12Surface.h"
#include "D3D12CommandQueue.h"

class D3D12F2FSync : public IRHIF2FSync
{
public:
    D3D12F2FSync(D3D12Device* device, D3D12Surface* surface, D3D12CommandQueue* queue);
    ~D3D12F2FSync();

    uint BeginSynchronize() override;
    void EndSynchronize(IRHICommandList* submitBuffer) override;
    void PresentSurface() override;
private:
    D3D12Device* mParentDevice;
    D3D12Surface* mSurface;
    D3D12CommandQueue* mCommandQueue;

    ID3D12Fence* mFence;
    StaticArray<uint64, FRAMES_IN_FLIGHT> mFrameValues;
    uint mFrameIndex;
};
