//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:22:04
//

#pragma once

#include "KGPU_CommandList.h"

namespace KGPU
{
    class ISync
    {
    public:
        virtual ~ISync() = default;

        virtual uint BeginSynchronize() = 0;
        virtual void EndSynchronize(ICommandList* submitBuffer) = 0;
        virtual void PresentSurface() = 0;
    };
}
