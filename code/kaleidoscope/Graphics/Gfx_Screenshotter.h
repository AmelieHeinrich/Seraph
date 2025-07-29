//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 22:42:30
//

#pragma once

#include "Gfx_Manager.h"
#include "Gfx_ResourceManager.h"

namespace Gfx
{
    class Screenshotter
    {
    public:
        static const uint MAX_WIDTH = 1920;
        static const uint MAX_HEIGHT = 1080;

        static void Initialize();
        static void Shutdown();

        static void EnqueueScreenshot(const char* screenshotID);
        static bool WantsScreenshot();
        static void ProcessScreenshot();
    private:
        static struct Data {
            bool WantsScreenshot;
            const char* ScreenshotResourceID;

            KGPU::IBuffer* ReadbackBuffer;
        } sData;
    };
}
