//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:46:25
//

#pragma once

#include <RHI/Surface.h>

class MetalDevice;
class MetalCommandQueue;

class MetalSurface : public IRHISurface
{
public:
    MetalSurface(MetalDevice* device, Window* window, MetalCommandQueue* commandQueue);
    ~MetalSurface();

private:
    MetalDevice* mParentDevice;
};
