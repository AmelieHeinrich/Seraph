//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 01:13:26
//

#pragma once

#include <KernelOS/KOS_Window.h>

namespace ToolImGui
{
    class Input
    {
    public:
        static void Initialize(KOS::IWindow* window);
        static void Shutdown();

        static void NewFrame();
        static void ReadEvent(void* event);
    };
}
