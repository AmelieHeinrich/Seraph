//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:11:33
//

#include "DummyBufferView.h"
#include "DummyDevice.h"

DummyBufferView::DummyBufferView(DummyDevice* device, RHIBufferViewDesc desc)
    : mParentDevice(device)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Dummy buffer view");
}

DummyBufferView::~DummyBufferView()
{
    
}
