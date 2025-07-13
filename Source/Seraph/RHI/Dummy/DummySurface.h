//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:46:25
//

#pragma once

#include <RHI/Surface.h>

class DummyDevice;
class DummyCommandQueue;

class DummySurface : public IRHISurface
{
public:
    DummySurface(DummyDevice* device, Window* window, DummyCommandQueue* commandQueue);
    ~DummySurface() override;

private:
    DummyDevice* mParentDevice;
};
