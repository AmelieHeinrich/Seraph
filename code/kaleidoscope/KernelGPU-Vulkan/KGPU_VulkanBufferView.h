//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:52:17
//

#pragma once

#include <KernelGPU/KGPU_BufferView.h>

namespace KGPU
{
    class VulkanDevice;

    class VulkanBufferView : public IBufferView
    {
    public:
        VulkanBufferView(VulkanDevice* device, BufferViewDesc desc);
        ~VulkanBufferView();

    private:
        VulkanDevice* mParentDevice;
    };
}
