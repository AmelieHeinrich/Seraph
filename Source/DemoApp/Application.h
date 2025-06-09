//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:22:34
//

#pragma once

#include <Seraph/Seraph.h>
#include <imgui/imgui.h>

#include "Camera.h"
#include "Renderer/Renderer.h"

struct ApplicationSpecs
{
    RHIBackend Backend;
    int WindowWidth;
    int WindowHeight;
};

class Application
{
public:
    Application(const ApplicationSpecs& specs);
    ~Application();

    void Run();
private:
    void BeginDockspace();
    void UI(RenderPassBegin& begin, RendererResource& ldr);
    void EndDockspace();

    ApplicationSpecs mSpecs;
    Camera mCamera;

    SharedPtr<Window> mWindow;

    IRHIDevice* mDevice;
    IRHICommandQueue* mGraphicsQueue;
    IRHISurface* mSurface;
    StaticArray<IRHICommandList*, FRAMES_IN_FLIGHT> mCommandBuffers;
    IRHIF2FSync* mF2FSync;
    IRHIImGuiContext* mImGuiContext;

    Renderer* mRenderer;

    ImVec2 mViewportSize;
    ImVec2 mViewportBounds[2];
    bool mViewportFocused = false;
    bool mGizmoFocused = false;
};
