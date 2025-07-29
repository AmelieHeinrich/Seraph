//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:33:34
//

#pragma once

#include "KGPU_CommandQueue.h"

#include <Agility/d3d12.h>

namespace KGPU
{
    class D3D12Device;

    class D3D12CommandQueue : public ICommandQueue
    {
    public:
        D3D12CommandQueue(D3D12Device* device, CommandQueueType type);
        ~D3D12CommandQueue();

        ICommandList* CreateCommandList(bool singleTime) override;
        void CommitCommandList(ICommandList* cmdBuffer) override;
        void Wait() override;
    public:
        static D3D12_COMMAND_LIST_TYPE TranslateToD3D12List(CommandQueueType type);

        ID3D12CommandQueue* GetQueue() { return mQueue; }

    private:
        D3D12Device* mParentDevice;
        ID3D12CommandQueue* mQueue = nullptr;
    };
}
