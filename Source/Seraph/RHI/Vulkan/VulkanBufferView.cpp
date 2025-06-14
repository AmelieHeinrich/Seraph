//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 12:08:21
//

#include "VulkanBufferView.h"
#include "VulkanDevice.h"

VulkanBufferView::VulkanBufferView(VulkanDevice* device, RHIBufferViewDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    switch (desc.Type)
    {
        case RHIBufferViewType::kConstant: {
            mBindless.Index = mParentDevice->GetBindlessManager()->WriteBufferCBV(this);
            break;
        }
        case RHIBufferViewType::kStructured: {
            mBindless.Index = mParentDevice->GetBindlessManager()->WriteBufferSRV(this);
            break;
        }
        case RHIBufferViewType::kStorage: {
            mBindless.Index = mParentDevice->GetBindlessManager()->WriteBufferUAV(this);
            break;
        }
    }

    SERAPH_WHATEVER("Created Vulkan buffer view");
}

VulkanBufferView::~VulkanBufferView()
{
    mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mBindless.Index);
}
