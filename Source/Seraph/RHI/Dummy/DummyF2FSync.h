//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:55:41
//

#pragma once

#include <RHI/F2FSync.h>

class DummySurface;
class DummyCommandQueue;
class DummyDevice;

class DummyF2FSync : public IRHIF2FSync
{
public:
    DummyF2FSync(DummyDevice* device, DummySurface* surface, DummyCommandQueue* queue);
    ~DummyF2FSync();

    uint BeginSynchronize() override;
    void EndSynchronize(IRHICommandList* submitBuffer) override;
    void PresentSurface() override;
private:
    DummyDevice* mParentDevice;
    DummySurface* mSurface;
    DummyCommandQueue* mCommandQueue;
};
