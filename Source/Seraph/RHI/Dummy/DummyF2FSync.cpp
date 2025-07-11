//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:56:27
//

#include "DummyF2FSync.h"
#include "DummyDevice.h"
#include "DummyCommandQueue.h"
#include "DummyCommandList.h"

DummyF2FSync::DummyF2FSync(DummyDevice* device, DummySurface* surface, DummyCommandQueue* queue)
    : mParentDevice(device), mSurface(surface), mCommandQueue(queue)
{
    SERAPH_WHATEVER("Created Dummy F2F sync");
}

DummyF2FSync::~DummyF2FSync()
{
}

uint DummyF2FSync::BeginSynchronize()
{
    return 0;
}

void DummyF2FSync::EndSynchronize(IRHICommandList* submitBuffer)
{

}

void DummyF2FSync::PresentSurface()
{

}
