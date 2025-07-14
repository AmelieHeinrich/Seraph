//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-14 20:52:20
//

#pragma once

#include <Seraph/RHI/Device.h>

class Screenshotter
{
public:
    static void Initialize(IRHIDevice* device, IRHICommandQueue* queue, uint width, uint height);
    static void Shutdown();

    static void EnqueueScreenshot();
    static bool WantsScreenshot();
    static void ProcessScreenshot();
private:
    static struct Data {
        uint Width;
        uint Height;
        bool WantsScreenshot;

        IRHIDevice* Device;
        IRHICommandQueue* Queue;
        IRHIBuffer* ReadbackBuffer;
    } sData;
};
