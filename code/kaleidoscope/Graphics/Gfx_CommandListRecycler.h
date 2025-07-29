//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 18:36:44
//

#pragma once

#include "Gfx_Manager.h"

namespace Gfx
{
    class CommandListRecycler
    {
    public:
        static void Clean();

        static KGPU::ICommandList* RequestCommandList();
        static void FlushCommandLists();
    private:
        static struct Data {
            KGPU::ICommandList* mActiveList = nullptr;
            bool mRecording = false;
        } sData;
    };
}
