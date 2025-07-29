//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:53:20
//

#include "KGPU_VulkanBufferView.h"
#include "KGPU_VulkanDevice.h"

namespace KGPU
{
    VulkanBufferView::VulkanBufferView(VulkanDevice* device, BufferViewDesc desc)
        : mParentDevice(device)
    {
        mDesc = desc;

        switch (desc.Type) {
            case BufferViewType::kConstant: {
                mBindless.Index = mParentDevice->GetBindlessManager()->WriteBufferCBV(this);
                break;
            }
            case BufferViewType::kStructured: {
                mBindless.Index = mParentDevice->GetBindlessManager()->WriteBufferSRV(this);
                break;
            }
            case BufferViewType::kStorage: {
                mBindless.Index = mParentDevice->GetBindlessManager()->WriteBufferUAV(this);
                break;
            }
        }

        KD_WHATEVER("Created Vulkan buffer view");
    }

    VulkanBufferView::~VulkanBufferView()
    {
        mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mBindless.Index);
    }
}
