//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 23:00:33
//

#pragma once

#include <RHI/ImGuiContext.h>
#include <Core/Window.h>

class DummyDevice;
class DummyCommandQueue;

class DummyImGuiContext : public IRHIImGuiContext
{
public:
    DummyImGuiContext(DummyDevice* device, DummyCommandQueue* queue, Window* window);
    ~DummyImGuiContext();

private:
    DummyDevice* mParentDevice;
};
