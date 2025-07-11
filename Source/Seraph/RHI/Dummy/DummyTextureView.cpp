//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:53:40
//

#include "DummyTextureView.h"
#include "DummyDevice.h"

DummyTextureView::DummyTextureView(DummyDevice* device, RHITextureViewDesc viewDesc)
    : mParentDevice(device)
{
    mDesc = viewDesc;

    SERAPH_WHATEVER("Created Dummy texture view");
}

DummyTextureView::~DummyTextureView()
{

}
