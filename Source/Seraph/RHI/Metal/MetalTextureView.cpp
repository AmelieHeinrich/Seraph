//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:53:40
//

#include "MetalTextureView.h"
#include "MetalDevice.h"

MetalTextureView::MetalTextureView(MetalDevice* device, RHITextureViewDesc viewDesc)
    : mParentDevice(device)
{
    mDesc = viewDesc;

    SERAPH_WHATEVER("Created Metal texture view");
}

MetalTextureView::~MetalTextureView()
{

}
