//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:11:33
//

#include "MetalBufferView.h"
#include "MetalDevice.h"

MetalBufferView::MetalBufferView(MetalDevice* device, RHIBufferViewDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Metal buffer view");
}

MetalBufferView::~MetalBufferView()
{
    
}
