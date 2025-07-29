//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:31:36
//

#pragma once

#include <KernelCore/KC_Context.h>

namespace KGPU
{
    class IDevice;
    class ICommandList;

    enum class CommandQueueType
    {
        kGraphics,
        kCompute,
        kCopy,
        kVideoEncode,
        kVideoDecode
    };

    class ICommandQueue
    {
    public:
        virtual ~ICommandQueue() = default;

        virtual ICommandList* CreateCommandList(bool singleTime) = 0;
        virtual void CommitCommandList(ICommandList* cmdBuffer) = 0;
        virtual void Wait() = 0;

        CommandQueueType GetType() const { return mType; }
    protected:
        CommandQueueType mType;
    };
}
