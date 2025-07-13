//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 23:00:33
//

#pragma once

#include <RHI/ImGuiContext.h>
#include <Core/Window.h>

class MetalDevice;
class MetalCommandQueue;

class MetalImGuiContext : public IRHIImGuiContext
{
public:
    MetalImGuiContext(MetalDevice* device, MetalCommandQueue* queue, Window* window);
    ~MetalImGuiContext() override;

private:
    MetalDevice* mParentDevice;
};
