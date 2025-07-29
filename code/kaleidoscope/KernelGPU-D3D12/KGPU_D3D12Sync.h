//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:26:51
//

#pragma once

#include <KernelGPU/KGPU_Sync.h>

#include "KGPU_D3D12Surface.h"
#include "KGPU_D3D12CommandQueue.h"

namespace KGPU
{
    class D3D12Sync : public ISync
    {
    public:
        D3D12Sync(D3D12Device* device, D3D12Surface* surface, D3D12CommandQueue* queue);
        ~D3D12Sync();

        uint BeginSynchronize() override;
        void EndSynchronize(ICommandList* submitBuffer) override;
        void PresentSurface() override;
    private:
        D3D12Device* mParentDevice;
        D3D12Surface* mSurface;
        D3D12CommandQueue* mCommandQueue;

        ID3D12Fence* mFence;
        uint64 mFrameValues[FRAMES_IN_FLIGHT];
        uint mFrameIndex;
    };
}
