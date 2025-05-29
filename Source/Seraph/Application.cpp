//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

Application::Application()
{
    mWindow = SharedPtr<Window>(new Window(1280, 720, "Seraph"));
    
    mDevice = IRHIDevice::CreateDevice(RHIBackend::kVulkan, true);
    mSurface = mDevice->CreateSurface(mWindow.get());
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
}

Application::~Application()
{
    delete mGraphicsQueue;
    delete mSurface;
    delete mDevice;
}

void Application::Run()
{
    while (mWindow->IsOpen()) {
        mWindow->PollEvents();
    }
}
