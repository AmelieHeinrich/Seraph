//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:36:04
//

#include "KGPU_D3D12CommandQueue.h"
#include "KGPU_D3D12Device.h"
#include "KGPU_D3D12CommandList.h"

namespace KGPU
{
    DWORD AwaitFence(ID3D12Fence* fence, uint64 val, uint64 timeout)
    {
        DWORD result = WAIT_FAILED;
        if (fence->GetCompletedValue() < val) {
            HANDLE eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
            fence->SetEventOnCompletion(val, eventHandle);
            if (eventHandle != 0) {
                result = WaitForSingleObject(eventHandle, timeout);
                CloseHandle(eventHandle);
            }
        } else {
            result = WAIT_OBJECT_0;
        }
        return result;
    }

    DWORD AwaitQueue(ID3D12Device* device, ID3D12CommandQueue* queue, uint64 timeout)
    {
        ID3D12Fence1* fence = nullptr;
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        if (!fence) {
            return WAIT_FAILED;
        }
        queue->Signal(fence, 1);
        auto result = AwaitFence(fence, 1, timeout);
        fence->Release();
        return result;
    }

    D3D12CommandQueue::D3D12CommandQueue(D3D12Device* device, CommandQueueType type)
        : mParentDevice(device)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = TranslateToD3D12List(type);

        HRESULT result = device->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(&mQueue));
        KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 command queue!");
    }

    D3D12CommandQueue::~D3D12CommandQueue()
    {
        if (mQueue) mQueue->Release();
    }

    ICommandList* D3D12CommandQueue::CreateCommandList(bool singleTime)
    {
        return (KC_NEW(D3D12CommandList, mParentDevice, this, singleTime));
    }

    void D3D12CommandQueue::CommitCommandList(ICommandList* cmdBuffer)
    {
        ID3D12CommandList* lists[] = { static_cast<D3D12CommandList*>(cmdBuffer)->GetList() };
        mQueue->ExecuteCommandLists(1, lists);
        
        AwaitQueue(mParentDevice->GetDevice(), mQueue, UINT64_MAX);
    }

    void D3D12CommandQueue::Wait()
    {
        AwaitQueue(mParentDevice->GetDevice(), mQueue, UINT64_MAX);
    }

    D3D12_COMMAND_LIST_TYPE D3D12CommandQueue::TranslateToD3D12List(CommandQueueType type)
    {
        switch (type)
        {
            case CommandQueueType::kGraphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
            case CommandQueueType::kCompute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case CommandQueueType::kCopy: return D3D12_COMMAND_LIST_TYPE_COPY;
            case CommandQueueType::kVideoEncode: return D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;
            case CommandQueueType::kVideoDecode: return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;
        }
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
}
