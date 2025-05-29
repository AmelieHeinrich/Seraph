//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:22:34
//

#pragma once

#include "Core/Window.h"
#include "RHI/Device.h"

class Application
{
public:
    Application();
    ~Application();

    void Run();
private:
    SharedPtr<Window> mWindow;

    IRHIDevice* mDevice;
    IRHISurface* mSurface;
};
