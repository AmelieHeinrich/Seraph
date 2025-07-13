//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:55:41
//

#pragma once

#include <RHI/F2FSync.h>

class MetalSurface;
class MetalCommandQueue;
class MetalDevice;

class MetalF2FSync : public IRHIF2FSync
{
public:
    MetalF2FSync(MetalDevice* device, MetalSurface* surface, MetalCommandQueue* queue);
    ~MetalF2FSync() override;

    uint BeginSynchronize() override;
    void EndSynchronize(IRHICommandList* submitBuffer) override;
    void PresentSurface() override;
private:
    MetalDevice* mParentDevice;
    MetalSurface* mSurface;
    MetalCommandQueue* mCommandQueue;

    uint mFrameIndex = 0;
};
