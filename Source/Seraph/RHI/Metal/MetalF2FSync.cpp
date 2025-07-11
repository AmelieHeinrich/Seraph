//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:56:27
//

#include "MetalF2FSync.h"
#include "MetalDevice.h"
#include "MetalCommandQueue.h"
#include "MetalCommandList.h"

MetalF2FSync::MetalF2FSync(MetalDevice* device, MetalSurface* surface, MetalCommandQueue* queue)
    : mParentDevice(device), mSurface(surface), mCommandQueue(queue)
{
    SERAPH_WHATEVER("Created Metal F2F sync");
}

MetalF2FSync::~MetalF2FSync()
{
}

uint MetalF2FSync::BeginSynchronize()
{
    return 0;
}

void MetalF2FSync::EndSynchronize(IRHICommandList* submitBuffer)
{

}

void MetalF2FSync::PresentSurface()
{

}
