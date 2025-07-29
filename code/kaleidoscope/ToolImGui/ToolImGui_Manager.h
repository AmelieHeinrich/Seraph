//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-17 21:12:35
//

#pragma once

#include <KernelOS/KOS_Window.h>
#include <KernelGPU/KGPU_Device.h>

#include "ToolImGui_Renderer.h"

namespace ToolImGui
{
    class Manager
    {
    public:
        static void Initialize(KOS::IWindow* window, KGPU::IDevice* device);
        static void Shutdown();

        static void BuildRenderer();

        static void Update(void* event);
        static void Begin();
        static void Render(KGPU::ICommandList* commandList, int frameIndex);
    private:
        static struct Data {
            KOS::IWindow* Window;
            KGPU::IDevice* Device;

            Renderer* GuiRenderer;
        } sData;
    };
}
