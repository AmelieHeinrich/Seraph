//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 14:46:19
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

namespace Gfx
{
    class Manager
    {
    public:
        static void Initialize(KGPU::IDevice* device, KGPU::ICommandQueue* cmdQueue) {
            sData.Device = device;
            sData.CommandQueue = cmdQueue;
        }

        static KGPU::IDevice* GetDevice() { return sData.Device; }
        static KGPU::ICommandQueue* GetCommandQueue() { return sData.CommandQueue; }
    private:
        static struct Data {
            KGPU::IDevice* Device;
            KGPU::ICommandQueue* CommandQueue;
        } sData;
    };
}
