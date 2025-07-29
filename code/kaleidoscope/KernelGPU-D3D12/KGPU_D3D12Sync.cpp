//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:27:36
//

#include "KGPU_D3D12Sync.h"
#include "KGPU_D3D12Device.h"
#include "KGPU_D3D12CommandQueue.h"
#include "KGPU_D3D12CommandList.h"

namespace KGPU
{
    D3D12Sync::D3D12Sync(D3D12Device* device, D3D12Surface* surface, D3D12CommandQueue* queue)
    : mParentDevice(device), mSurface(surface), mCommandQueue(queue)
    {
        HRESULT result = device->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
        KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 fence!");

        KD_WHATEVER("Created D3D12 F2F sync");
    }

    D3D12Sync::~D3D12Sync()
    {
        if (mFence) mFence->Release();
    }

    uint D3D12Sync::BeginSynchronize()
    {
        mFrameIndex = mSurface->GetSwapchain()->GetCurrentBackBufferIndex(); 
        return mFrameIndex;
    }

    void D3D12Sync::EndSynchronize(ICommandList* submitBuffer)
    {
        ID3D12CommandList* lists[] = { static_cast<D3D12CommandList*>(submitBuffer)->GetList() };
        mCommandQueue->GetQueue()->ExecuteCommandLists(1, lists);

        uint64 fenceValue = mFrameValues[mFrameIndex];
        mCommandQueue->GetQueue()->Signal(mFence, fenceValue);

        if (mFence->GetCompletedValue() < mFrameValues[mFrameIndex]) {
            HANDLE event = ::CreateEventA(nullptr, false, false, "Fence Wait Event");
            mFence->SetEventOnCompletion(mFrameValues[mFrameIndex], event);
            if (::WaitForSingleObject(event, UINT64_MAX) == WAIT_TIMEOUT) {
                KD_ASSERT_EQ(false, "!! GPU TIME-OUT !!");
            }
        }
        mFrameValues[mFrameIndex] = fenceValue + 1;
    }

    void D3D12Sync::PresentSurface()
    {
        mSurface->GetSwapchain()->Present(0, 0);
    }
}
